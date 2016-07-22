#include "queryclass.h"
#include "tasks/queryinit.h"
#include "algo/algo.h"
#include "tasks/processor.h"
#include "tasks/receiver.h"
#include "configuration.h"


using namespace std;

extern Configuration *config;

namespace vanet {


QueryClass::QueryClass(QObject *parent, MainWindow *mainWindowParent, Scheduling sched, int topK, vector<int> classes) : QObject(parent)
{
    this->mainWindowParent = mainWindowParent;
    this->sched = sched;
    this->topK = topK;
    this->classes = classes;
}



void QueryClass::runQuery() {
    vector<string> client_list;
    devices.clear();
    /*if (client_list.size() == 0) {
        client_list.push_back("192.168.1.210");
        client_list.push_back("192.168.1.250");
        //client_list.push_back("10.100.1.201");
        //client_list.push_back("10.100.1.202");
        //client_list.push_back("10.100.1.203");
    }*/

    taskCount = 0;

    for(string client : config->getDevices()) {
        QueryInit *init = new QueryInit(0, this, client);
        QThread *t = newTaskThread(init);
        connect(init, SIGNAL(deviceAdded(MobileDevice*)), this, SLOT(deviceAdded(MobileDevice*)));
        connect(t, SIGNAL(finished()), this, SLOT(initFinished()));
        taskCount++;
        t->start();
    }
}

void QueryClass::initFinished() {

    if(--taskCount != 0)
        return;

    activeCount = 1;
    prepareQuery();

    vector<QThread*> receivers;
    for (MobileDevice *dev : devices) {
        Receiver *recv = new Receiver(0, dev);
        QThread* thread = new QThread;
        recv->moveToThread(thread);
        connect(thread, SIGNAL(started()), recv, SLOT(run()));
        connect(recv, SIGNAL(shutdown()), thread, SLOT(quit()));
        connect(recv, SIGNAL(shutdown()), recv, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(recv, SIGNAL(videoReceived(Video*)), this, SLOT(videoReceived(Video*)));
        connect(recv, SIGNAL(finished()), this, SLOT(receiverFinished()));
        connect(this, SIGNAL(finalUpload(MobileDevice*,std::set<Video*>*)), recv, SLOT(finalUpload(MobileDevice*,std::set<Video*>*)));
        connect(recv, SIGNAL(finalUploadFinished()), this, SLOT(finalUploadFinished()));
        connect(this, SIGNAL(shutdownReceiver(MobileDevice*)), recv, SLOT(shutdownReceiver(MobileDevice*)));
        connect(recv, SIGNAL(updateVideo(Video*)), mainWindowParent, SLOT(updateVideoStatus(Video*)));
        unique_lock<mutex> lck(countLock);
        taskCount++;
        lck.unlock();
        thread->start();
    }

    if (!procQueue.empty()) {
        activeCount++;
        Processor *processor = new Processor(0, topK);
        procThread = newTaskThread(processor);
        connect(processor, SIGNAL(processFinished(Video*)), this, SLOT(processFinished(Video*)));
        connect(this, SIGNAL(process(Video*)), processor, SLOT(process(Video*)), Qt::QueuedConnection);
        connect(this, SIGNAL(shutdownProcessor()), processor, SLOT(shutdown()));
        connect(processor, SIGNAL(finished()), this, SLOT(finish()));

        procThread->start();
        sendVideoRequest(procQueue.front());
    }
    emit startTimer();


}

void QueryClass::receiverFinished() {
    unique_lock<mutex> lck(countLock);
    taskCount--;
    cout << "Receivers remaining: " << taskCount << endl;
    if (taskCount != 0) {
        lck.unlock();
        return;
    }
    lck.unlock();
    cout << "Receivers finished\n";
    finish();
}

void QueryClass::finish() {
    unique_lock<mutex> lck(countLock);
    if (--activeCount != 0) {
        lck.unlock();
        return;
    }
    lck.unlock();
    emit queryFinished();
    for (MobileDevice *dev : devices) {
        vanet_pb::ServerMessage smsg;
        set<Video*> vids;
        for (Video *vid : dev->get_videos()) {
            if (vid->mode == vanet_pb::ProcessMode::MOBILE && vid->is_match(classes)) {
                smsg.add_path(vid->path);
                vids.insert(vid);
                //cout << "Requesting upload: " << vid->name << endl;
            }
        }
        if (true /*smsg.path_size() == 0*/) {
            smsg.set_type(vanet_pb::ServerMessage::DISCONNECT);
            cout << "Shutdown receiver: " << dev->get_ip() << endl;
            emit shutdownReceiver(dev);
            //delete dev;
        }
        else {
            smsg.set_type(vanet_pb::ServerMessage::VIDEO_REQUEST);
            cout << "Final upload: " << dev->get_ip() << endl;
            taskCount++;
            emit finalUpload(dev, &vids);
        }

        dev->send_message(smsg);
    }
    if (taskCount == 0) {
        cout << "QUERY FINISHED\n";
        cout.flush();
        emit finished();
    }
}

void QueryClass::finalUploadFinished() {
    unique_lock<mutex> lck(countLock);
    if (--taskCount != 0) {
        lck.unlock();
        return;
    }
    lck.unlock();
    cout << "QUERY FINISHED\n";
    cout.flush();
    emit finished();
}

QThread* QueryClass::newTaskThread(QObject *task) {
    QThread* thread = new QThread;
    task->moveToThread(thread);
    connect(thread, SIGNAL(started()), task, SLOT(run()));
    connect(task, SIGNAL(finished()), thread, SLOT(quit()));
    connect(task, SIGNAL(finished()), task, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    return thread;
}

void QueryClass::deviceAdded(MobileDevice *dev) {
    unique_lock<mutex> lck(lock);
    devices.push_back(dev);
    lck.unlock();
    emit addDevice(dev);
}

void QueryClass::prepareQuery() {
    int size = devices.size();

    vector<node*> nodes;
    queue<Video*>().swap(procQueue);
    procCount = 0;
    mobileCount = 0;
    serverCount = 0;

    double mobile_speed = config->getMobileSpeed(); //1000000. / 0.5; //.79
    double server_speed = config->getServerSpeed(); //1000000. / 0.0035;
    double transfer_speed = config->getTransferSpeed() * 1000000.;


    switch (sched) {
    case OPT:
        for(MobileDevice *dev : devices) {
            map<int,double> map_rates;
            //map_rates[0] = test_speed(c_messages[i].first);
            map_rates[size] = transfer_speed;
            node *n = new node(dev, mobile_device, mobile_speed, map_rates);
            for (Video *vid : dev->get_videos()) {
                n->videos.push_back(videos_tuple(vid->size, 0., vid));
            }
            nodes.push_back(n);
        }
        nodes.push_back(new node(video_cloud, server_speed));
        for (int i = 0; i < nodes.size(); i++) {
            nodes[i]->sort();
        }

        greedy_alg(nodes, size, 1);

        for (int i = 0; i < size; i++) {
            MobileDevice *dev = devices[i];
            node *n = nodes[i];
            vanet_pb::ServerMessage msg;
            msg.set_type(vanet_pb::ServerMessage::PROCESS_DIRECTIVE);
            msg.set_top_k(topK);
            for (int tag : classes)
                msg.add_tags(tag);

            for (videos_tuple t : n->videos) {
                Video *vid = get<2>(t);
                vid->mode = vanet_pb::MOBILE;
                msg.add_path(vid->path);
                msg.add_process_mode(vid->mode);
                mobileCount++;
            }

            for (videosto_tuple t : n->videosto) {
                Video *vid = get<2>(t);
                vid->mode = vanet_pb::SERVER;
                msg.add_path(vid->path);
                msg.add_process_mode(vid->mode);
                serverCount++;
            }

            dev->send_message(msg);
        }

        for (videosfrom_tuple t : nodes[size]->videosfrom) {
            Video *vid = get<2>(t);
            procQueue.push(vid);
            procCount++;
        }

        break;
        
        // Figure out how to store Videos in MobileDevice
    case MOBILE:
        for(MobileDevice *dev : devices) {
            vanet_pb::ServerMessage smsg;
            smsg.set_type(vanet_pb::ServerMessage::PROCESS_DIRECTIVE);
            smsg.set_top_k(topK);
            for(Video *vid : dev->get_videos()) {
                vid->mode = vanet_pb::ProcessMode::MOBILE;
                smsg.add_path(vid->path);
                smsg.add_process_mode(vid->mode);
                mobileCount++;
            }
            dev->send_message(smsg);
        }
        break;

    case SERVER:
        for(MobileDevice *dev : devices) {
            vanet_pb::ServerMessage smsg;
            smsg.set_type(vanet_pb::ServerMessage::PROCESS_DIRECTIVE);
            smsg.set_top_k(topK);
            for(Video *vid : dev->get_videos()) {
                vid->mode = vanet_pb::ProcessMode::SERVER;
                procQueue.push(vid);
                procCount++;
                smsg.add_path(vid->path);
                smsg.add_process_mode(vid->mode);
                serverCount++;
            }
            dev->send_message(smsg);
        }
        break;
    }

    emit updateCountLabels(serverCount, mobileCount);
}

void QueryClass::processFinished(Video *vid) {
    procCount--;
    vid->status = DONE;
    emit updateVideo(vid);
    if (procCount == 0 && !procThread->isFinished()) {
        emit shutdownProcessor();
        cout << "Processor shutting down\n";
    }
}

void QueryClass::videoReceived(Video *vid) {
    emit process(vid);
    if (vid == procQueue.front())
        procQueue.pop();
    if (!procQueue.empty())
        sendVideoRequest(procQueue.front());
    vid->status = PROCESSING_SERVER;
    emit updateVideo(vid);
}

void QueryClass::sendVideoRequest(Video *vid) {
    vanet_pb::ServerMessage msg;
    msg.set_type(vanet_pb::ServerMessage::VIDEO_REQUEST);
    msg.add_path(vid->path);
    vid->dev->send_message(msg);
    cout << "Video requested: " << vid->name << endl;
}

}

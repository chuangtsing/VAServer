#include "receiver.h"
#include <iostream>

using namespace std;

namespace vanet {
Receiver::Receiver(QObject *parent, MobileDevice *dev) : QObject(parent)
{
    this->dev = dev;
}

void Receiver::run() {
    bool connected = true;

    while (connected && !dev->remainingEmpty()) {
        vanet_pb::ClientMessage c_msg;
        if(!dev->receive_message(c_msg))
            continue;
        if(c_msg.type() == vanet_pb::ClientMessage::VIDEO_INFO) {
            vanet_pb::VideoInfo info = c_msg.video_info(0);
            cout << "Info received: " << info.name() << endl;
            Video *vid = dev->get_video(info.path());
            for (int tag : info.tags())
                vid->add_tag(tag);
            dev->removeRemaining(vid);
            vid->status = DONE;
            emit updateVideo(vid);
        }
        else if (c_msg.type() == vanet_pb::ClientMessage::VIDEO) {
            cout << "Receiving video: " << c_msg.video_info(0).name() << "\tSize: "
                 << c_msg.video_info(0).size() << endl;
            Video *vid = dev->get_video(c_msg.video_info(0).path());
            vid->status = UPLOADING;
            emit updateVideo(vid);
            string path = "../VAServer/vaserver/videos/uploaded/";
            long received = dev->receive_video(path, c_msg);
            vid->local_path = path;

            emit videoReceived(vid);
            dev->removeRemaining(vid);
        }
        cout << "RECV Videos remaining: " << dev->getRemaining() << endl;
    }
    cout << "Receiver finished\n";
    emit finished();
}

void Receiver::finalUpload(MobileDevice *dev, set<Video*> *vids) {
    cout << "Final upload signaled\n";
    if (this->dev != dev)
        return;
    cout << "Final upload started" << endl;
    while(!vids->empty()) {
        vanet_pb::ClientMessage c_msg;
        if(!dev->receive_message(c_msg))
            continue;
        else if (c_msg.type() == vanet_pb::ClientMessage::VIDEO) {
            cout << "Receiving video: " << c_msg.video_info(0).name() << "\tSize: "
                 << c_msg.video_info(0).size() << endl;
            Video *vid = dev->get_video(c_msg.video_info(0).path());
            string path = "../VAServer/vaserver/videos/uploaded/";
            long received = dev->receive_video(path, c_msg);
            vid->local_path = path;
            vids->erase(vid);
        }
    }
    emit finalUploadFinished();
    emit shutdown();
}

void Receiver::shutdownReceiver(MobileDevice *dev) {
    if (this->dev == dev) {
        emit shutdown();
        //delete dev;
    }
}
}

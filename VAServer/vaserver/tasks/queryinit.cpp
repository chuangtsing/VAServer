#include "queryinit.h"

using namespace std;

namespace vanet {
QueryInit::QueryInit(QObject *parent, QueryClass *queryClassParent, string ip) : QObject(parent)
{
    this->queryClassParent = queryClassParent;
    this->ip = ip;
}

void QueryInit::run() {
    MobileDevice *dev = new MobileDevice(ip, true);
    if (dev->get_socket() == -1) {
        emit finished();
        return;
    }

    vanet_pb::ServerMessage s_message;
    vanet_pb::ClientMessage c_message;

    s_message.set_type(vanet_pb::ServerMessage::QUERY_TAG);

    if(!dev->send_message(s_message)) {
        emit finished();
        return;
    }

    if(!dev->receive_message(c_message)) {
        emit finished();
        return;
    }

    for(vanet_pb::VideoInfo info : c_message.video_info())
        dev->add_video(new Video(info, dev));

    queryClassParent->deviceAdded(dev);
    emit finished();
}

}

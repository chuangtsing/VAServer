#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>
#include "mobile_device.h"

namespace vanet {
class Receiver : public QObject
{
    Q_OBJECT
public:
    explicit Receiver(QObject *parent = 0, MobileDevice *dev = NULL);

private:
    MobileDevice *dev;

signals:
    void finished();
    void videoReceived(Video *vid);
    void updateVideo(Video *vid);
    void shutdown();
    void finalUploadFinished();

public slots:
    void run();
    void finalUpload(MobileDevice *dev, std::set<Video*> *vids);
    void shutdownReceiver(MobileDevice *dev);

};
}
#endif // RECEIVER_H

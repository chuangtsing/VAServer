#ifndef QUERYCLASS_H
#define QUERYCLASS_H

#include <vector>
#include "mobile_device.h"
#include <string>
#include <mutex>
#include <QObject>
#include "mainwindow.h"
#include <queue>
#include <QThread>

namespace vanet {

typedef enum Scheduling {
    OPT, MOBILE, SERVER
} Scheduling;

class QueryClass : public QObject
{
    Q_OBJECT

public:
    explicit QueryClass(QObject *parent = 0, MainWindow *mainWindowParent = 0, Scheduling sched = OPT, int topK = 13, std::vector<int> classes = std::vector<int>());

private:
    std::vector<MobileDevice*> devices;
    QThread *newTaskThread(QObject *task);
    std::mutex lock;
    MainWindow *mainWindowParent;
    std::queue<Video*> procQueue;
    Scheduling sched;
    int topK;
    int taskCount;
    int activeCount;
    QThread *procThread;
    std::mutex countLock;
    int procCount;
    std::vector<int> classes;

    int mobileCount;
    int serverCount;

    void prepareQuery();
    void sendVideoRequest(Video *vid);


public slots:
    void runQuery();
    void initFinished();
    void deviceAdded(MobileDevice *dev);
    void processFinished(Video *vid);
    void videoReceived(Video *vid);
    void receiverFinished();
    void finalUploadFinished();
    void finish();

signals:
    void finished();
    void queryFinished();
    void addDevice(vanet::MobileDevice*);
    void updateVideo(Video *vid);
    void shutdownProcessor();
    void process(Video *vid);
    void shutdownReceiver(MobileDevice *dev);
    void finalUpload(MobileDevice *dev, std::set<Video*> *vids);
    void updateCountLabels(int server, int mobile);
    void startTimer();
};
}

#endif

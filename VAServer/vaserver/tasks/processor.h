#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QObject>
#include "mobile_device.h"
#include "vanet.h"
#include "classifier.h"
#include <memory>
#include <mutex>
#include <condition_variable>

namespace vanet {
class Processor : public QObject
{
    Q_OBJECT
public:
    explicit Processor(QObject *parent = 0, int topK = 13);
private:
    //static std::unique_ptr<caffe::Classifier> classifier;
    int topK;
    std::mutex lock;
    std::condition_variable cond;
    bool processing;
signals:
    void processFinished(Video *vid);
    void finished();

public slots:
    void run();
    void process(Video *vid);
    void shutdown();
};
}
#endif // PROCESSOR_H

#include "processor.h"
#include "vid_utils.h"

using namespace std;

caffe::Classifier *classifier;

namespace vanet {

//unique_ptr<caffe::Classifier> Processor::classifier;

Processor::Processor(QObject *parent, int topK) : QObject(parent)
{
    this->topK = topK;
}

void Processor::run() {
}


void Processor::process(Video *vid) {
    unique_lock<mutex> lck(lock);
    processing = true;
    lck.unlock();
    vector<cv::Mat*> imgs;
    extract_frames(vid, imgs);
    vector<caffe::Prediction> predictions = predict_frames(imgs, classifier, caffe::Caffe::GPU, topK);
    for (caffe::Prediction p : predictions) {
        vid->add_tag(p.first);
    }
    lck.lock();
    processing = false;
    lck.unlock();
    emit processFinished(vid);
}

void Processor::shutdown() {
    unique_lock<mutex> lck(lock);
    while (processing)
        cond.wait(lck);
    lck.unlock();
    emit finished();
}
}

#define BOOST_EXCEPTION_DISABLE
#include "mainwindow.h"
#include <QApplication>
#include "tasks/processor.h"
#include "configuration.h"
#include <string>

extern caffe::Classifier *classifier;

Configuration *config;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    config = new Configuration(std::string("../VAServer/config.xml"));
    classifier = new caffe::Classifier("../VAServer/vaserver/models/arl/model.prototxt", "../VAServer/vaserver/models/arl/weights.caffemodel",
                                                                                  "../VAServer/vaserver/models/arl/mean.binaryproto");
    w.show();
    int ret = a.exec();
    delete classifier;
	delete config;
    return ret;
}

#-------------------------------------------------
#
# Project created by QtCreator 2015-12-23T22:17:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VAServer
TEMPLATE = app


CAFFE = /export/home/zbl5008/caffe
CUDA  = /opt/cuda-7.0
BLAS = /opt/openblas



LIBRARIES += caffe m stdc++ rt openblas protobuf glog  opencv_highgui opencv_core opencv_imgproc boost_regex boost_system pthread
LIBS += -L$${CAFFE}/build/lib \
        -L$${BLAS} \
        -L/usr/lib \
        -L$${CUDA}/lib64 \
        -L/usr/lib/boost
LIBS += $(foreach library,$${LIBRARIES},-l${library})

INCLUDEPATH += $${CAFFE}/include /usr/include $${CUDA}/include $${BLAS} . vaserver

SOURCES += main.cpp \
        mainwindow.cpp \
        vaserver/vid_utils.cpp \
        vaserver/video.cpp \
        vaserver/classifier.cpp \
        #vaserver/task_handlers.cpp \
        vaserver/algo/algo.cpp \
        vaserver/algo/node.cpp \
        vaserver/queryclass.cpp \
        vaserver/mobile_device.cpp \
        vaserver/protobuf/vanet_pb.pb.cpp \
    vaserver/tasks/queryinit.cpp \
    vaserver/tasks/receiver.cpp \
    devicemodel.cpp \
    resultsmodel.h \
    vaserver/tasks/processor.cpp \
    resultsmodel.cpp \
    configuration.cpp

HEADERS  += mainwindow.h \
    vaserver/queryclass.h \
    vaserver/classifier.h \
    #vaserver/task_handlers.h \
    vaserver/vanet.h \
    vaserver/vid_utils.h \
    vaserver/video.h \
    vaserver/mobile_device.h \
    vaserver/protobuf/vanet_pb.pb.h \
    vaserver/algo/algo.h \
    vaserver/algo/node.h \
    vaserver/tasks/queryinit.h \
    vaserver/tasks/receiver.h \
    devicemodel.h \
    resultsmodel.h \
    vaserver/tasks/processor.h \
    resultsmodel.h \
    configuration.h

CONFIG += c++11

FORMS    += mainwindow.ui

#ifndef VID_UTILS_H
#define VID_UTILS_H

#include "classifier.h"
#include <string>
#include "video.h"

#define MODEL_PATH	"models/arl/model.prototxt"
#define WEIGHT_PATH "models/arl/weights.caffemodel"
#define MEAN_PATH	"models/arl/mean.binaryproto"
#define SYNSET_PATH	"models/arl/synset.txt"

#define VIDEOS_PATH "videos/"
#define FRAMES_PATH "videos/extracted/"
#define FRAMES_EXT "_frames"

std::string extract_frames(std::string filename);
bool extract_frames(Video *vid, std::vector<cv::Mat*> &imgs);
std::vector<caffe::Prediction> predict_frames(const std::vector<cv::Mat*> &imgs, caffe::Classifier *classifier, caffe::Caffe::Brew mode, int N);
#endif

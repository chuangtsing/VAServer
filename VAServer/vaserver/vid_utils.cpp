#include "classifier.h"
//#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <sys/stat.h>
#include "vanet.h"
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include "vid_utils.h"
#include <dirent.h>
#include <regex>
#include <thread>
#include <iostream>

using namespace std;
using namespace vanet;

bool extract_frames(Video *vid, vector<cv::Mat*> &imgs) {
	//double fps;
    vector<int> params;
	int end;
	cv::Mat img;
	cv::Mat *mat;

    cv::VideoCapture cap(/*"videos/" + */vid->local_path);
	if (!cap.isOpened()) {
        printf("Unable to open video: %s\n", vid->name.c_str());
		return false;
	}

	end = cap.get(CV_CAP_PROP_FRAME_COUNT);
    printf("EXTRACTION %s: %d\n\n", vid->name.c_str(), end);
	
	params.push_back(CV_IMWRITE_JPEG_QUALITY);
	params.push_back(100);


	int i = 1;
	int frame = 0;
    while (cap.read(img)) {
		mat = new cv::Mat(img.clone());
		imgs.push_back(mat);
		i++;
		frame += 30;
		if (frame > end)
			break;
        cap.set(CV_CAP_PROP_POS_FRAMES, frame);
    }
    cap.release();

    printf("EXTRACTION FINISHED: %s\n", vid->name.c_str());

	return true;
}


std::vector<caffe::Prediction> predict_frames(const vector<cv::Mat*> &imgs, caffe::Classifier *classifier, caffe::Caffe::Brew mode, int N) {
	std::vector<float> total;
	for (auto img : imgs) {
		CHECK(!img->empty()) << "Unable to decode image";
		classifier->ClassifyCollective(*img, mode, total);
		delete img;
	}

	std::vector<caffe::Prediction> predictions = classifier->CompileCollective(total, N);
	/*// Print the top N predictions.
	for (size_t i = 0; i < predictions.size(); ++i) {
		caffe::Prediction p = predictions[i];
		std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
		<< p.first << "\"" << std::endl;
	}*/
	
	return predictions;

}

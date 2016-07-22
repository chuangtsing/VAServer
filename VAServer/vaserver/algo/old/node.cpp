//
//  node.h
//  schedule
//
//  Created by Zongqing Lu on 7/8/15.
//  Copyright (c) 2015 Zongqing Lu. All rights reserved.
//
#include "node.hpp"
#include <algorithm>


using namespace std;
    
node::node(int sock, int node_type, double speed){
	socket = sock;
	type = node_type;
	delay = 0.;
	pspeed = speed;
}
    
node::node(int sock, int node_type, double speed, map<int,double> mapRates){
	socket = sock;
	type = node_type;
	delay = 0.;
	pspeed = speed;
	rates = mapRates;
}
    
void node::sort(){
	if(videos.size())
		std::sort(videos.begin(), videos.end(), compare<long,double,string>);
}
    
double node::getCTime(){
	double ctime = 0.;
	vector<double> vecTime;
	for (int i = 0; i < videos.size(); i++) {
		double ctime = 0.;
		videos_tuple video = videos[i];
		double cdelay = get<1>(video);
		for (int j = 0; j < videos.size(); j++) {
			if(get<1>(videos[j]) >= cdelay){
				ctime += get<0>(videos[j])/pspeed;
			}
		}
		ctime += cdelay;
		vecTime.push_back(ctime);
	}
	std::sort(vecTime.begin(), vecTime.end());
	if(!vecTime.size())
		ctime = 0;
	else
		ctime = vecTime.back();
	return ctime;
}
    
double node::testCTime(videos_tuple video){
	double ctime = 0.;
	videos.push_back(video);
	ctime = getCTime();
	videos.pop_back();
	return ctime;
}
    
videos_tuple node::allocateVideoFrom(int vindex, int cindex, double cdelay){
	if(vindex < 0 || vindex >= videos.size() || videos.size() == 0)
		return 0;
	videos_tuple video = videos[vindex];
	videos.erase(videos.begin()+vindex);
	delay = cdelay;
	videosto.push_back(videosto_tuple(get<0>(video), cindex, get<2>(video)));
	return video;
}
    
void node::allocateVideoTo(long size, double cdelay, int from, const string &name){
	videos.push_back(videos_tuple(size, cdelay, name));
	delay = cdelay;
	videosfrom.push_back(videosfrom_tuple(size, from, name));
}
    
double node::getMaxRate(){
	double maxrate = 0.;
	for (auto i = rates.begin(); i != rates.end(); i++) {
		if (i->second > maxrate)
			maxrate = i->second;
	}
	return maxrate;
}


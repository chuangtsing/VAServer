//
//  node.h
//  schedule
//
//  Created by Zongqing Lu on 7/8/15.
//  Copyright (c) 2015 Zongqing Lu. All rights reserved.
//

#ifndef schedule_node_h
#define schedule_node_h

#include <vector>
#include <map>
#include <random>
#include <ctime>

enum{
    mobile_device = 0,
    video_cloud
};

template <typename T1, typename T2, typename T3>
bool compare(std::tuple<T1, T2, T3> x, std::tuple<T1, T2, T3> y){
    return std::get<0>(x) < std::get<0>(y);
};

typedef std::tuple<long,double,std::string> videos_tuple;
typedef std::tuple<long,int,std::string> videosto_tuple;
typedef std::tuple<long,int,std::string> videosfrom_tuple;

class node{

private:
    int socket;
    int type;
    double delay;
    double pspeed;
public:
	std::vector<videos_tuple> videos; // (size, cdelay, path)
	std::vector<videosto_tuple> videosto;   //(size, cloud index, vidintex) // at mobile devices
	std::vector<videosfrom_tuple> videosfrom; //(size, mobile index, path)   at server
	std::map<int, double> rates;
    
    node(int sock, int node_type, double speed);
    
    node(int sock, int node_type, double speed, std::map<int,double> mapRates);
    
    void sort();
    
    double getCTime();
    
    double testCTime(videos_tuple video);
    
    videos_tuple allocateVideoFrom(int vindex, int cindex, double cdelay);
    
    void allocateVideoTo(long size, double cdelay, int from, const std::string &name);
    
    int getSocket(){
        return socket;
    }
    
    int getType(){
        return type;
    }
    
    double getDelay(){
        return delay;
    }
    
    double getSpeed(){
        return pspeed;
    }
    
    double getRate(int index){
        return rates[index];
    }
    
    double getMaxRate();
    
};

#endif

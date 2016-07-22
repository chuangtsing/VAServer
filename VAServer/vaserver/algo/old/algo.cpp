//
//  main.cpp
//  schedule
//
//  Created by Zongqing Lu on 7/8/15.
//  Copyright (c) 2015 Zongqing Lu. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "node.hpp"
#include "algo.hpp"


#define DPRATE      2.0
#define CPRATE      100.0
#define TRATE       16.0
#define VARIATION   0.6

using namespace std;

void greedy_alg(vector<node *> & vecNodes, int nod, int noc){
    int i;
    double T_opt = 0;
    while (true) {
        double tmp = 0;
        double ratio = 0;
        int cnt = 0;
        for (i = 0; i < nod; i++) {
            if(vecNodes[i]->getCTime() >= T_opt){
                tmp += vecNodes[i]->getCTime() * vecNodes[i]->getSpeed() / vecNodes[i]->getMaxRate();
                ratio += vecNodes[i]->getSpeed() / vecNodes[i]->getMaxRate();
                ++cnt;
            }
        }
        for (i = nod; i < noc + nod; i++) {
            tmp += vecNodes[i]->getCTime();
        }
        
        tmp = tmp / (ratio + noc);
        if(tmp == T_opt)
            break;
        else
            T_opt = tmp;
    }
    
    //-----------------------------------------------------------------
    //greedy algorithm
    
    while (true) {
        //------------------------------------------------------------------
        //calculate T
        double atime = 0;
        int tspeed = 0;
        for(i = 0; i < vecNodes.size(); i++)
            tspeed += vecNodes[i]->getSpeed();
        
        double tload = 0;
        int argmax = 0;
        double mcompletion = 0;
        
        for(i=0; i<vecNodes.size(); i++){
            if(mcompletion < vecNodes[i]->getCTime()){
                mcompletion = vecNodes[i]->getCTime();
                argmax = i;
            }
            tload += vecNodes[i]->getCTime() * vecNodes[i]->getSpeed();
        }
        
        atime = tload/tspeed;
        //------------------------------------------------------------------
        if(vecNodes[argmax]->getType() == video_cloud)
            break;
        
        bool realloated = false;
        
        node * pnode = vecNodes[argmax];
        for(i = (int)pnode->videos.size() - 1; i >= 0; i--){
            if(get<0>(pnode->videos[i]) <= (mcompletion - atime) * pnode->getSpeed()){
                vector<tuple<double, int, string>> values;
                for (int j = nod; j < nod + noc; j++) {
                    double otime = vecNodes[j]->getCTime();
                    double tmp = get<0>(pnode->videos[i])/pnode->getRate(j) + max(pnode->getDelay(), vecNodes[j]->getDelay());
                    double dtime = vecNodes[j]->testCTime(videos_tuple(get<0>(pnode->videos[i]), tmp, get<2>(pnode->videos[i]));
                    dtime = dtime - otime;
                    if(dtime + otime <= atime){
                        values.push_back(tuple<double, int, string>(dtime, j, get<2>(pnode->videos[i])));
                    }
                }
                if(values.size() > 0){
                    sort(values.begin(), values.end(),compare<double, int>);
                    double cdelay = get<0>(pnode->videos[i])/pnode->getRate(get<1>(values.begin())) + max(pnode->getDelay(), vecNodes[values.begin()->second]->getDelay());
                    videos_tuple vid_tuple = pnode->allocateVideoFrom(i, values.begin()->second, cdelay, get<0>(pnode->videos[i])); //remove video from mobile_device
                    vecNodes[get<1>(values.begin())]->allocateVideoTo(get<0>(vid_tuple), cdelay, argmax, get<2>(vid_tuple)); //add video to video_cloud
                    realloated = true;
                    break;
                }
            }
        }
        if(!realloated){
			videos_tuple v_tuple = *pnode->videos.begin();
            long size = get<0>v_tuple>
            vector<tuple<double,int,string> values;
            for (int j = nod; j < nod + noc; j++) {
                double ctime = vecNodes[j]->testCTime(videos_tuple(size,
							size/pnode->getRate(j) + max(pnode->getDelay(),vecNodes[j]->getDelay())), get<2>(v_tuple));
                if(ctime <= mcompletion)
                    values.push_back(tuple<double,int,string>(ctime, j, get<2>(v_tuple)));
            }
            if(!values.size()) break;  // no video reallocation can improve completion time
            sort(values.begin(), values.end(),compare<double, int>);
            double cdelay = size/pnode->getRate(get<1>(*values.begin())) + max(pnode->getDelay(), vecNodes[get<1>(*values.begin())]->getDelay());
            pnode->allocateVideoFrom(0,get<1>(*values.begin()), cdelay);
            vecNodes[get<1>(*values.begin())]->allocateVideoTo(size, cdelay, argmax);
        }
    }
    
    vector<pair<double, int>> vecRet;
    for(i=0; i<vecNodes.size(); i++){
        for (int j = 0; j < vecNodes[i]->videos.size(); j++) {
            cout << vecNodes[i]->videos[j].first << ", " << vecNodes[i]->videos[j].second << "; ";
        }
        cout << endl;
        cout << "process time after: " << vecNodes[i]->getCTime() << endl;
        vecRet.push_back(pair<double, int>(vecNodes[i]->getCTime(), i));
    }
    sort(vecRet.begin(), vecRet.end(), compare<double, int>);
    cout << "T_max: " << vecRet.back().first << "; Index: " << vecRet.back().second << endl;
    cout << "T_opt: " << T_opt << endl;
}


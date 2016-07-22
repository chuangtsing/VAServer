//
//  main.cpp
//  schedule
//
//  Created by Zongqing Lu on 7/8/15.
//  Copyright (c) 2015 Zongqing Lu. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "node.h"
#include <sstream>


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
                    double dtime = vecNodes[j]->testCTime(videos_tuple(get<0>(pnode->videos[i]), tmp, get<2>(pnode->videos[i])));
                    dtime = dtime - otime;
                    if(dtime + otime <= atime){
                        values.push_back(tuple<double, int, string>(dtime, j, get<2>(pnode->videos[i])));
                    }
                }
                if(values.size() > 0){
                    sort(values.begin(), values.end(),compare<double, int, string>);
                    double cdelay = get<0>(pnode->videos[i])/pnode->getRate(get<1>(*values.begin())) + max(pnode->getDelay(), vecNodes[get<1>(*values.begin())]->getDelay());
                    videos_tuple vidt = pnode->allocateVideoFrom(i, get<1>(*values.begin()), cdelay); //remove video from mobile_device
                    vecNodes[get<1>(*values.begin())]->allocateVideoTo(get<0>(vidt), cdelay, argmax, get<2>(vidt)); //add video to video_cloud
                    realloated = true;
                    break;
                }
            }
        }
        if(!realloated){
            int size = get<0>(*pnode->videos.begin());
            vector<tuple<double, int, string>> values;
            for (int j = nod; j < nod + noc; j++) {
                double ctime = vecNodes[j]->testCTime(videos_tuple(size, size/pnode->getRate(j) + max(pnode->getDelay(),vecNodes[j]->getDelay()), get<2>(*pnode->videos.begin())));
                if(ctime <= mcompletion)
                    values.push_back(tuple<double, int, string>(ctime, j, get<2>(*pnode->videos.begin())));
            }
            if(!values.size()) break;  // no video reallocation can improve completion time
            sort(values.begin(), values.end(),compare<double, int, string>);
            double cdelay = size/pnode->getRate(get<1>(*values.begin())) + max(pnode->getDelay(), vecNodes[get<1>(*values.begin())]->getDelay());
            pnode->allocateVideoFrom(0, get<1>(*values.begin()), cdelay);
            vecNodes[get<1>(*values.begin())]->allocateVideoTo(size, cdelay, argmax, get<2>(*values.begin()));
        }
    }
    
    vector<pair<double, int>> vecRet;
    for(i=0; i<vecNodes.size(); i++){
        for (int j = 0; j < vecNodes[i]->videos.size(); j++) {
            cout << get<2>(vecNodes[i]->videos[j]) << ", " << get<1>(vecNodes[i]->videos[j]) << "; ";
        }
        cout << endl;
        cout << "process time after: " << vecNodes[i]->getCTime() << endl;
        vecRet.push_back(pair<double, int>(vecNodes[i]->getCTime(), i));
    }
    sort(vecRet.begin(), vecRet.end(), compare<double, int>);
    cout << "T_max: " << vecRet.back().first << "; Index: " << vecRet.back().second << endl;
    cout << "T_opt: " << T_opt << endl;
}

void show_usage(string name){
    cerr << "Usage: " << name << " <option(s)> "
    << "Options:\n"
    << "\t-h,--help\t\tShow this help message\n"
    << "\t-c,--cloud\t\tSpecify the number of videos clouds\n"
    << "\t-d,--device\t\tSpecify the number of mobile devices\n"
    << "\t-v,--video\t\tSpecify the number of videos\n"
    << std::endl;
}

int main(int argc, const char * argv[]) {
    int noc = 0;
    int nod = 0;
    int nov = 0;
    
    double trate = TRATE;
    double pcspeed = CPRATE;
    double pdspeed = DPRATE;
    
    vector<double> vec_rates;
    
    if (argc < 7) {
        show_usage(argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        } else if ((arg == "-c") || (arg == "--cloud")) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                noc = atoi(argv[++i]); // Increment 'i' so we don't get the argument as the next argv[i].
            } else { // Uh-oh, there was no argument to the destination option.
                std::cerr << "--cloud option requires one argument." << std::endl;
                return 1;
            }
        } else if((arg == "-d") || (arg == "--device")) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                nod = atoi(argv[++i]); // Increment 'i' so we don't get the argument as the next argv[i].
            } else { // Uh-oh, there was no argument to the destination option.
                std::cerr << "--device option requires one argument." << std::endl;
                return 1;
            }
        } else if((arg == "-v") || (arg == "--video")) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                nov = atoi(argv[++i]); // Increment 'i' so we don't get the argument as the next argv[i].
            } else { // Uh-oh, there was no argument to the destination option.
                std::cerr << "--video option requires one argument." << std::endl;
                return 1;
            }
        }
    }
    
    if (!nod) {
        std::cerr << "please specify the number of mobile devices using option -d or --device." << std::endl;
        return 1;
    }
    if (!noc) {
        std::cerr << "please specify the number of video clouds using option -c or --cloud." << std::endl;
        return 1;
    }
    if (!nov) {
        std::cerr << "please specify the number of videos using option -v or --video." << std::endl;
        return 1;
    }
    
    sort(vec_rates.begin(), vec_rates.end());
    
    
    ofstream output;
    output.open("/Users/zongqing/Documents/output.txt", ofstream::out);
    output.close();
    
    
    default_random_engine generator((random_device())());
    normal_distribution<double> normal(50, 20);
    uniform_int_distribution<int> uniform(0, noc+nod-1);
    uniform_real_distribution<double> dist_pdspeed((double)pdspeed * VARIATION, pdspeed);
    uniform_real_distribution<double> dist_pcspeed((double)pcspeed * VARIATION, pcspeed);
    uniform_real_distribution<double> dist_rate((double)trate * VARIATION, trate);
    
    auto dice_video = bind (normal, generator);
    auto dice_node = bind(uniform, generator);
    auto dice_pdspeed = bind(dist_pdspeed, generator);
    auto dice_pcspeed = bind(dist_pcspeed, generator);
    auto dice_rate = bind(dist_rate, generator);
    
    
    vector<node *> vecNodes_g;
    int i;
    
    for (i = 0; i < nod; i++)
    {
        map<int, double> mapRates;
        for (int j = nod; j < nod + noc; j++) {
            mapRates[j] = dice_rate(); // Transfer rate
        }
        double speed = dice_pdspeed(); // Device process speed
        node * p = new node(i, mobile_device, speed, mapRates); // speed and transmission rate MB/s
        vecNodes_g.push_back(p);
    }
    
    for (i = 0; i < noc; i++)
    {
        double speed = dice_pcspeed(); // Server process speed
        node * p = new node(nod+i, video_cloud, speed);
        vecNodes_g.push_back(p);
    }
    
    // Add videos to nodes
    i = 0;
    while (true) {
        int size = dice_video();
        if( size >= 0 && size <= 100)
        {
            int which = dice_node();
			stringstream ss;
			ss << "Vid_" << i;
            vecNodes_g[0]->videos.push_back(videos_tuple(size, 0., ss.str())); 
            ++i;
        }
        if(i == nov) break;
    }
    
    
    for (i = 0; i < vecNodes_g.size(); i++) {
        vecNodes_g[i]->sort();
    }
    
    for(i=0; i<vecNodes_g.size(); i++){
        for (int j = 0; j < vecNodes_g[i]->videos.size(); j++) {
            cout << i << ": " << get<2>(vecNodes_g[i]->videos[j]) << ", " << get<1>(vecNodes_g[i]->videos[j]) << "; ";
        }
        cout << endl;
        cout << "process time before: " << vecNodes_g[i]->getCTime() << endl;
    }

    
    greedy_alg(vecNodes_g, nod, noc);
    
    
    for(i=0; i<vecNodes_g.size(); i++){
        delete vecNodes_g[i];
    }

    return 0;
}

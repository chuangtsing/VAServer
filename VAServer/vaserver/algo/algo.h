//
//  main.cpp
//  schedule
//
//  Created by Zongqing Lu on 7/8/15.
//  Copyright (c) 2015 Zongqing Lu. All rights reserved.
//
#ifndef ALGO_H
#define ALGO_H

#include "node.h"


#define DPRATE      2.0
#define CPRATE      100.0
#define TRATE       2.0
#define VARIATION   0.6

void greedy_alg(std::vector<node *> & vecNodes, int nod, int noc);

#endif

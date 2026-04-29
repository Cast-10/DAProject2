//
// Created by ze on 29/04/26.
//

#ifndef DAPROJECT2_BUILD_H
#define DAPROJECT2_BUILD_H

#include "Graph.h"
#include "RegisterAlloc.h"

vector<Web> buildWebs(const vector<LiveRange> &ranges);

Graph<int> buildInterferenceGraph(const vector<Web> &webs);

#endif //DAPROJECT2_BUILD_H

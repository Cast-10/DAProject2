#ifndef DAPROJECT2_BUILD_H
#define DAPROJECT2_BUILD_H

#include "Graph.h"
#include "LiveRange.h"
#include "Web.h"

using namespace std;

vector<Web> buildWebs(const vector<LiveRange> &ranges);

Graph<int> buildInterferenceGraph(const vector<Web> &webs);

#endif //DAPROJECT2_BUILD_H

#ifndef DAPROJECT2_BUILD_H
#define DAPROJECT2_BUILD_H

#include "Graph.h"
#include "LiveRange.h"
#include "Web.h"

using namespace std;

/**
 * @brief Builds webs by merging overlapping live ranges.
 *
 * @param ranges Vector containing all live ranges.
 *
 * @return Vector of constructed webs.
 *
 * @complexity
 * Time: O(R * R), with R standing for number of live ranges.
 */
vector<Web> buildWebs(const vector<LiveRange> &ranges);

/**
 * @brief Builds the Interference Graph.
 * Two nodes (webs) have an edge if they "interfere"
 * Two nodes "interfere" if they share a point in the program, except
 * when one point ends with '-' and the other starts with '+' on the same line
 *
 * @param webs Vector of webs.
 *
 * @return Interference graph.
 *
 * @complexity
 * Time: O(W * W), where W is the number of webs.
 */
Graph<int> buildInterferenceGraph(const vector<Web> &webs);

#endif //DAPROJECT2_BUILD_H

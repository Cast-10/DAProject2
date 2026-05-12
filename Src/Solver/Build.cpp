#include "Build.h"
#include <algorithm>
#include <map>

/**
 * Build Webs from LiveRanges
 */
vector<Web> buildWebs(const vector<LiveRange> &ranges) {
    vector<Web> webs;

    std::map<string, vector<LiveRange>> rangesByVar;
    for (const auto& lr : ranges) {
        rangesByVar[lr.varName].push_back(lr);
    }

    int nextWebId = 0;
    for (auto& entry : rangesByVar) {
        const string& varName = entry.first;
        vector<LiveRange>& varRanges = entry.second;

        vector<bool> visited(varRanges.size(), false); //Check witch ranges were evaluated already, init all false

        for (size_t i = 0; i < varRanges.size(); ++i) {
            if (visited[i]) continue;

            Web currentWeb(nextWebId++, varName);

            vector<size_t> toProcess;
            toProcess.push_back(i);
            visited[i] = true;

            size_t head = 0;
            while(head < toProcess.size()){
                size_t currIdx = toProcess[head++];
                currentWeb.merge(varRanges[currIdx]);

                for (size_t j = 0; j < varRanges.size(); ++j) {
                    if (!visited[j]) {
                        if (varRanges[currIdx].overlapsWith(varRanges[j])) {
                            visited[j] = true;
                            toProcess.push_back(j);
                        }
                    }
                }
            }
            webs.push_back(currentWeb);
        }
    }

    return webs;
}

/**
 * Build Interference Graph
 * Two nodes (webs) have an edge if they "interfere"
 * Two nodes "interfere" if they share a point in the program, except
 * when one point ends with '-' and the other starts with '+' on the same line
 */
Graph<int> buildInterferenceGraph(const vector<Web> &webs) {
    Graph<int> g;

    for (const auto& w : webs) {
        g.addVertex(w.id);
    }

    // Check interference between all pairs of webs (O(N^2)) N being the number of webs, put lane in html!
    for (size_t i = 0; i < webs.size(); ++i) {
        for (size_t j = i + 1; j < webs.size(); ++j) {
            if (webs[i].interferesWith(webs[j])) {
                g.addEdge(webs[i].id, webs[j].id, 1.0);
                g.addEdge(webs[j].id, webs[i].id, 1.0);  // edges in both directions because it is undirected graph
            }
        }
    }

    return g;
}
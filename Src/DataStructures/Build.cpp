//
// Created by ze on 29/04/26.
//

#include "Build.h"

/*
 Merge live ranges of the same variable into webs.
 Two live ranges of the same variable are merged if they share
 at least one line number (they overlap or touch).
 A special case from the spec: if range A ends at line X (isLastUse)
 and range B starts at line X (isDef), they are also fused.

 Uses a simple union-find / greedy approach:
   - start with each live range as its own candidate web
   - repeatedly merge any two that share a line
*/
vector<Web> buildWebs(const vector<LiveRange> &ranges) {
    // Group ranges by variable name
    map<string, vector<LiveRange>> byVar;
    for (auto &lr : ranges)
        byVar[lr.varName].push_back(lr);

    vector<Web> webs;
    int webId = 0;

    for (auto &[varName, lrs] : byVar) {
        // Each live range starts as its own "candidate web" (set of points)
        vector<set<int>> lineSets;
        vector<vector<ProgramPoint>> pointSets;

        for (auto &lr : lrs) {
            set<int> s;
            for (auto &p : lr.points) s.insert(p.lineNum);
            lineSets.push_back(s);
            pointSets.push_back(lr.points);
        }

        // Merge until no more merges happen
        bool merged = true;
        while (merged) {
            merged = false;
            for (size_t i = 0; i < lineSets.size(); i++) {
                for (size_t j = i + 1; j < lineSets.size(); j++) {
                    // Check overlap: shared line numbers
                    bool overlap = false;
                    for (int line : lineSets[i])
                        if (lineSets[j].count(line)) { overlap = true; break; }

                    // Check touching: end of i == start of j or vice versa
                    // (the "i=i+1" case from the spec)
                    if (!overlap) {
                        // find the max line of i that has isLastUse
                        for (auto &pi : pointSets[i]) {
                            if (pi.isLastUse) {
                                for (auto &pj : pointSets[j]) {
                                    if (pj.isDef && pj.lineNum == pi.lineNum)
                                        { overlap = true; break; }
                                }
                            }
                            if (overlap) break;
                        }
                    }
                    if (!overlap) {
                        for (auto &pj : pointSets[j]) {
                            if (pj.isLastUse) {
                                for (auto &pi : pointSets[i]) {
                                    if (pi.isDef && pi.lineNum == pj.lineNum)
                                        { overlap = true; break; }
                                }
                            }
                            if (overlap) break;
                        }
                    }

                    if (overlap) {
                        // Merge j into i
                        for (int line : lineSets[j])  lineSets[i].insert(line);
                        for (auto &p  : pointSets[j]) {
                            // Only add point if line not already present
                            bool found = false;
                            for (auto &pi : pointSets[i])
                                if (pi.lineNum == p.lineNum) { found = true; break; }
                            if (!found) pointSets[i].push_back(p);
                        }
                        sort(pointSets[i].begin(), pointSets[i].end());

                        // Remove j
                        lineSets.erase(lineSets.begin() + j);
                        pointSets.erase(pointSets.begin() + j);

                        merged = true;
                        break;
                    }
                }
                if (merged) break;
            }
        }

        // Each remaining candidate becomes a Web
        for (auto &pts : pointSets) {
            Web w;
            w.id      = webId++;
            w.varName = varName;
            w.points  = pts;
            webs.push_back(w);
        }
    }

    return webs;
}


// Build the interference graph from a list of webs.
// Nodes = web IDs (int), undirected edges between interfering webs.

Graph<int> buildInterferenceGraph(const vector<Web> &webs) {
    Graph<int> g;

    // Add one vertex per web
    for (auto &w : webs)
        g.addVertex(w.id);

    // Add undirected edges between interfering webs
    for (size_t i = 0; i < webs.size(); i++) {
        for (size_t j = i + 1; j < webs.size(); j++) {
            if (webs[i].interferesWith(webs[j])) {
                g.addEdge(webs[i].id, webs[j].id, 1.0);
                g.addEdge(webs[j].id, webs[i].id, 1.0); // undirected
            }
        }
    }

    return g;
}

#include "RegisterAlloc.h"
#include "Build.h"
#include "MutablePriorityQueue.h"
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <climits>

using namespace std;

/**
 * Basic register allocation via Chaitin-style graph coloring simplification.
 *
 * Phase 1 – Simplification:
 *   Repeatedly remove vertices whose current degree < numRegisters, pushing
 *   them onto a stack. If the graph empties completely, a K-coloring exists.
 *   If we get stuck (every remaining vertex has degree >= K), allocation is
 *   infeasible and we report failure without spilling.
 *
 * Phase 2 – Coloring:
 *   Pop vertices from the stack and assign the smallest register index not
 *   already taken by any neighbor in the original interference graph.
 */
AllocationResult basicAllocation(const vector<Web>& webs, const Graph<int>& ig, const AlgorithmConfig& config) {
    AllocationResult result;
    result.webs = webs;
    int n = (int)webs.size();
    result.assignment.assign(n, -1);

    if (n == 0) {
        result.feasible = true;
        result.registersUsed = 0;
        return result;
    }

    // Build adjacency sets from the interference graph
    vector<set<int>> adj(n);
    for (auto* v : ig.getVertexSet()) {
        int u = v->getInfo();
        for (const auto& e : v->getAdj())
            adj[u].insert(e->getDest()->getInfo());
    }

    // Phase 1: simplification
    vector<int> degree(n);
    for (int i = 0; i < n; i++)
        degree[i] = (int)adj[i].size();

    vector<bool> removed(n, false);
    vector<int> stk;

    queue<int> q;
    for (int i = 0; i < n; i++) {
        if (degree[i] < config.numRegisters) {
            removed[i] = true;
            q.push(i);
        }
    }

    while (!q.empty()) {
        int u = q.front(); q.pop();
        stk.push_back(u);

        for (int nb : adj[u]) {
            if (!removed[nb]) {
                degree[nb]--;
                if (degree[nb] < config.numRegisters) {
                    removed[nb] = true;
                    q.push(nb);
                }
            }
        }
    }

    // If any vertex was never removed the graph cannot be K-colored
    for (int i = 0; i < n; i++) {
        if (!removed[i]) {
            result.feasible = false;
            result.registersUsed = 0;
            return result;
        }
    }

    // Phase 2: color in reverse simplification order
    int maxReg = 0;
    while (!stk.empty()) {
        int webId = stk.back();
        stk.pop_back();

        set<int> forbidden;
        for (int nb : adj[webId]) {
            if (result.assignment[nb] >= 0)
                forbidden.insert(result.assignment[nb]);
        }

        int reg = 0;
        while (forbidden.count(reg)) reg++;

        result.assignment[webId] = reg;
        maxReg = max(maxReg, reg);
    }

    result.registersUsed = maxReg + 1;
    result.feasible = true;
    return result;
}

/**
 * Register allocation via graph coloring with controlled spilling.
 *
 * Extension of basicAllocation: when simplification gets stuck (every
 * remaining node has degree >= numRegisters), instead of failing immediately
 * we spill the highest-degree node to memory and continue. We repeat this
 * up to maxSpill times. If the graph is still uncolorable after maxSpill
 * spills, we report failure.
 *
 * Spill heuristic — highest current degree:
 *   The node with the most live-range conflicts is the one most likely to
 *   block further simplification. Removing it collapses the largest number
 *   of edges at once, giving adjacent nodes the best chance to fall below
 *   the degree threshold and become simplifiable. This is equivalent to the
 *   "remove the densest node" strategy used in Chaitin-style allocators.
 *
 * Spilled webs receive assignment -1 (memory) and are excluded from the
 * coloring phase. result.feasible is true whenever the remaining graph is
 * successfully K-colored, even if spills occurred; it is false only when
 * the budget is exhausted and coloring is still impossible.
 */
AllocationResult spillingAllocation(const vector<Web>& webs, const Graph<int>& ig,
                                    const AlgorithmConfig& config) {
    AllocationResult result;
    result.webs = webs;
    int n = (int)webs.size();
    result.assignment.assign(n, -1);

    if (n == 0) {
        result.feasible = true;
        result.registersUsed = 0;
        return result;
    }

    // Build adjacency sets from the interference graph
    vector<set<int>> adj(n);
    for (auto* v : ig.getVertexSet()) {
        int u = v->getInfo();
        for (const auto& e : v->getAdj())
            adj[u].insert(e->getDest()->getInfo());
    }

    vector<int>  degree(n);
    for (int i = 0; i < n; i++)
        degree[i] = (int)adj[i].size();

    vector<bool> removed(n, false);
    vector<bool> spilled(n, false);
    vector<int>  stk;
    int spillCount = 0;

    // Phase 1: simplification with spilling fallback
    while (true) {
        // Remove every node whose current degree < numRegisters
        bool any = true;
        while (any) {
            any = false;
            for (int i = 0; i < n; i++) {
                if (removed[i] || degree[i] >= config.numRegisters) continue;
                removed[i] = true;
                stk.push_back(i);
                for (int nb : adj[i])
                    if (!removed[nb]) degree[nb]--;
                any = true;
            }
        }

        // Check if the graph is empty
        bool allDone = true;
        for (int i = 0; i < n; i++)
            if (!removed[i]) { allDone = false; break; }
        if (allDone) break;

        // Stuck: spill the node with the highest current degree
        if (spillCount >= config.param) {
            result.feasible = false;
            result.registersUsed = 0;
            return result;
        }

        int best = -1;
        for (int i = 0; i < n; i++) {
            if (removed[i]) continue;
            if (best == -1 || degree[i] > degree[best]) best = i;
        }

        removed[best] = true;
        spilled[best] = true;
        spillCount++;
        for (int nb : adj[best])
            if (!removed[nb]) degree[nb]--;
    }

    // Phase 2: color in reverse simplification order; spilled nodes keep -1
    int maxReg = -1;
    while (!stk.empty()) {
        int webId = stk.back();
        stk.pop_back();

        set<int> forbidden;
        for (int nb : adj[webId])
            if (!spilled[nb] && result.assignment[nb] >= 0)
                forbidden.insert(result.assignment[nb]);

        int reg = 0;
        while (forbidden.count(reg)) reg++;

        result.assignment[webId] = reg;
        maxReg = max(maxReg, reg);
    }

    result.registersUsed = (maxReg >= 0) ? maxReg + 1 : 0;
    result.feasible = true;
    return result;
}

// --- Splitting helpers ---

static void reindexWebs(vector<Web>& webs) {
    for (int i = 0; i < (int)webs.size(); i++)
        webs[i].id = i;
}

// Split web w after pts[splitIdx]: first half ends with '-', second begins with '+'
static pair<Web, Web> doSplit(const Web& w, int splitIdx, int id1, int id2) {
    vector<pair<int,char>> pts(w.points.begin(), w.points.end());
    int splitLine = pts[splitIdx].first;

    Web h1(id1, w.varName);
    h1.displayName = w.displayName + ".1";
    for (int i = 0; i <= splitIdx; i++) {
        int line = pts[i].first;
        h1.points[line] = (line == splitLine) ? '-' : pts[i].second;
    }

    Web h2(id2, w.varName);
    h2.displayName = w.displayName + ".2";
    for (int i = splitIdx; i < (int)pts.size(); i++) {
        int line = pts[i].first;
        h2.points[line] = (line == splitLine) ? '+' : pts[i].second;
    }

    return {h1, h2};
}

// For web at webIdx, find the split index that minimises max(deg(h1), deg(h2))
static int bestSplitIdx(const Web& w, const vector<Web>& webs, int webIdx) {
    vector<pair<int,char>> pts(w.points.begin(), w.points.end());
    int n = (int)pts.size();

    int best = 1, bestCost = INT_MAX;
    for (int si = 1; si < n - 1; si++) {
        int splitLine = pts[si].first;

        Web h1(0, w.varName), h2(0, w.varName);
        for (int i = 0; i <= si; i++) {
            int line = pts[i].first;
            h1.points[line] = (line == splitLine) ? '-' : pts[i].second;
        }
        for (int i = si; i < n; i++) {
            int line = pts[i].first;
            h2.points[line] = (line == splitLine) ? '+' : pts[i].second;
        }

        int d1 = 0, d2 = 0;
        for (int j = 0; j < (int)webs.size(); j++) {
            if (j == webIdx) continue;
            if (h1.interferesWith(webs[j])) d1++;
            if (h2.interferesWith(webs[j])) d2++;
        }
        int cost = max(d1, d2);
        if (cost < bestCost) { bestCost = cost; best = si; }
    }
    return best;
}

/**
 * Register allocation via graph coloring with controlled web splitting.
 *
 * Tries basic allocation first. When the graph is not K-colorable, selects
 * the highest-degree web and splits it at the point that minimises the maximum
 * degree of the two resulting halves. Repeats up to maxSplit (config.param)
 * times. The split point becomes a '-' (save) in the first half and a '+'
 * (reload) in the second half; by the def/use non-interference rule the two
 * halves never interfere with each other.
 */
AllocationResult splittingAllocation(const vector<Web>& inputWebs, const Graph<int>& /*ig*/,
                                     const AlgorithmConfig& config) {
    vector<Web> webs = inputWebs;
    int maxSplits = config.param;

    for (int splitsUsed = 0; splitsUsed <= maxSplits; splitsUsed++) {
        reindexWebs(webs);
        Graph<int> ig = buildInterferenceGraph(webs);
        AllocationResult result = basicAllocation(webs, ig, config);

        if (result.feasible) return result;
        if (splitsUsed == maxSplits) return result;

        // Degree of each web in the current interference graph
        int n = (int)webs.size();
        vector<int> degree(n, 0);
        for (auto* v : ig.getVertexSet())
            degree[v->getInfo()] = (int)v->getAdj().size();

        // Choose the highest-degree splittable web (>= 3 points, so si >= 1 is valid)
        int webIdx = -1;
        for (int i = 0; i < n; i++) {
            if ((int)webs[i].points.size() < 3) continue;
            if (webIdx == -1 || degree[i] > degree[webIdx]) webIdx = i;
        }
        if (webIdx == -1) return result; // no splittable web

        int si = bestSplitIdx(webs[webIdx], webs, webIdx);
        int nextId = n; // will be reindexed next iteration
        auto [h1, h2] = doSplit(webs[webIdx], si, nextId, nextId + 1);

        webs.erase(webs.begin() + webIdx);
        webs.push_back(h1);
        webs.push_back(h2);
    }

    AllocationResult fail;
    fail.webs = webs;
    fail.feasible = false;
    return fail;
}

AllocationResult freeAllocation(const vector<Web>& inputWebs, const Graph<int>& ig,
                                const AlgorithmConfig& config) {
    AllocationResult result;
    result.webs = inputWebs;
    int n = (int)inputWebs.size();
    result.assignment.assign(n, -1);

    if (n == 0) {
        result.feasible = true;
        result.registersUsed = 0;
        return result;
    }

    vector<vector<bool>> colors(n, vector<bool>(config.numRegisters, false));

    MutablePriorityQueue<Vertex<int>> mpq;
    for (auto v : ig.getVertexSet()) {
        v->setIndegree((unsigned int)v->getAdj().size());
        v->setSatur(0);
        mpq.insert(v);
    }

    int maxReg = -1;
    while (!mpq.empty()) {
        Vertex<int>* top = mpq.extractMin();
        int webId = top->getInfo();

        int check = -1;
        for (int i = 0; i < config.numRegisters; i++) {
            if (!colors[webId][i]) {
                check = i;
                break;
            }
        }

        if (check == -1) {
            result.feasible = false;
            result.registersUsed = 0;
            return result;
        }

        result.assignment[webId] = check;
        maxReg = max(maxReg, check);

        for (auto edge : top->getAdj()) {
            auto dest = edge->getDest();
            if (result.assignment[dest->getInfo()] != -1) continue;

            if (!colors[dest->getInfo()][check]) {
                colors[dest->getInfo()][check] = true;
                dest->setSatur(dest->getSatur() + 1);
                mpq.decreaseKey(dest);
            }
        }
    }

    result.registersUsed = maxReg + 1;
    result.feasible = true;
    return result;
}
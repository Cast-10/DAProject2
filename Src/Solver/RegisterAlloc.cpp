#include "RegisterAlloc.h"
#include <vector>
#include <set>
#include <queue>
#include <algorithm>

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
            adj[u].insert(e.getDest()->getInfo());
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
            adj[u].insert(e.getDest()->getInfo());
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
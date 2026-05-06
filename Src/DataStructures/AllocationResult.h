#ifndef DAPROJECT2_ALLOCATIONRESULT_H
#define DAPROJECT2_ALLOCATIONRESULT_H

#include "Web.h"
#include <vector>
#include <string>

/**
 * @brief Holds the result of a register allocation run.
 *
 * After running any allocation algorithm, the result contains:
 *   - The list of webs (in order)
 *   - For each web, its assigned register index (>= 0) or -1 for memory (spilled)
 *   - The total number of distinct registers actually used
 *   - Whether the allocation was feasible within the given register limit
 */
struct AllocationResult {
    std::vector<Web> webs;          ///< All webs, ordered by id
    std::vector<int>  assignment;   ///< assignment[i] = register index for webs[i], -1 = memory
    int registersUsed;              ///< Number of distinct registers used (0 if infeasible)
    bool feasible;                  ///< True if all webs got a register

    AllocationResult() : registersUsed(0), feasible(false) {}

    /// Returns "rK" for register K, or "M" for memory
    std::string registerName(int webId) const {
        int reg = assignment[webId];
        if (reg < 0) return "M";
        return "r" + std::to_string(reg);
    }
};

#endif //DAPROJECT2_ALLOCATIONRESULT_H

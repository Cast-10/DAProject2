#ifndef DAPROJECT2_REGISTERALLOC_H
#define DAPROJECT2_REGISTERALLOC_H

#include "AllocationResult.h"
#include "Graph.h"
#include <vector>

/**
 * @brief Basic register allocation via Chaitin-style graph coloring simplification.
 *
 * Phase 1 – Simplification:
 *   Repeatedly remove vertices whose current degree < numRegisters, pushing
 *   them onto a stack. If the graph empties completely, a K-coloring exists.
 *   If we get stuck (every remaining vertex has degree >= K), allocation is
 *   infeasible and we report failure without spilling.
 * Phase 2 – Coloring:
 *   Pop vertices from the stack and assign the smallest register index not
 *   already taken by any neighbor in the original interference graph.
 *
 * @param webs Vector of webs to allocate.
 * @param ig Interference graph between webs.
 * @param config Allocation configuration containing the number of registers.
 *
 * @return Results containing:
 * - Register assignments for each web.
 * - Number of registers used.
 * - Feasibility status.
 *
 * @complexity
 * Time: O(V + E)
 */
AllocationResult basicAllocation(const std::vector<Web>& webs, const Graph<int>& ig,
                                 const AlgorithmConfig& config);
/**
 * @brief Register allocation via graph coloring with controlled spilling.
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
 *
 * @param webs Vector of webs to allocate.
 * @param ig Interference graph between webs.
 * @param config Allocation configuration containing the number of registers.
 *
 * @return AllocationResult containing assignments, feasibility,
 * spills, and register number.
 *
 * @complexity
 * Time: O(S * V + E), with S being the maximum number of spills,
 * V the number of webs and E the number of interference edges.
 */

AllocationResult spillingAllocation(const std::vector<Web>& webs, const Graph<int>& ig,
                                    const AlgorithmConfig& config);

/**
 @brief Register allocation via graph coloring with controlled web splitting.
 *
 * Tries basic allocation first. When the graph is not K-colorable, selects
 * the highest-degree web and splits it at the point that minimises the maximum
 * degree of the two resulting halves. Repeats up to maxSplit (config.param)
 * times. The split point becomes a '-' (save) in the first half and a '+'
 * (reload) in the second half; by the def/use non-interference rule the two
 * halves never interfere with each other.
 *
 * @param inputWebs Initial web set.
 *
 * @param ig .
 *
 * @param config Allocation configuration containing the number of registers.
 *
 * @return AllocationResult containing assignments and feasibility.
 *
 * @complexity
 */
AllocationResult splittingAllocation(const std::vector<Web>& webs, const Graph<int>& ig,
                                     const AlgorithmConfig& config);

/**
 * @brief
 *
 * @param inputWebs Initial web set.
 * @param ig Interference graph.
 * @param config Allocation configuration containing the number of registers.
 *
 * @return AllocationResult with assignments and feasibility status.
 *
 * @ecomplexity
 */
AllocationResult freeAllocation(const std::vector<Web>& webs, const Graph<int>& ig,
                                const AlgorithmConfig& config);

#endif //DAPROJECT2_REGISTERALLOC_H
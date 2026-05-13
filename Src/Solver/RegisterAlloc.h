#ifndef DAPROJECT2_REGISTERALLOC_H
#define DAPROJECT2_REGISTERALLOC_H

#include "AllocationResult.h"
#include "Graph.h"
#include <vector>

AllocationResult basicAllocation(const std::vector<Web>& webs, const Graph<int>& ig,
                                 const AlgorithmConfig& config);

AllocationResult spillingAllocation(const std::vector<Web>& webs, const Graph<int>& ig,
                                    const AlgorithmConfig& config);

AllocationResult splittingAllocation(const std::vector<Web>& webs, const Graph<int>& ig,
                                     const AlgorithmConfig& config);

#endif //DAPROJECT2_REGISTERALLOC_H
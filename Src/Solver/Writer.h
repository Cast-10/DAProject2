//
// Created by ze on 13/05/26.
//

#ifndef DAPROJECT2_WRITTER_H
#define DAPROJECT2_WRITTER_H
#include "AllocationResult.h"
#include "Web.h"

using namespace std;

/**
 * @brief Writes the register allocation results to an output file.
 *
 * @param path Path to the output file.
 * @param result Structure containing the allocation result,
 * webs, assignments, feasibility status and number of registers used.
 * @param config Configuration parameters for the allocation algorithm,
 * including the number of registers and algorithm type.
 *
 * @complexity
 * Time: O(R * W), where R is number of registers and W is number of webs.
 */
void writeOutput(const string& path, const AllocationResult& result,
                 const AlgorithmConfig& config);

#endif //DAPROJECT2_WRITTER_H

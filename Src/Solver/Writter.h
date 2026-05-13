//
// Created by ze on 13/05/26.
//

#ifndef DAPROJECT2_WRITTER_H
#define DAPROJECT2_WRITTER_H
#include "AllocationResult.h"
#include "Web.h"

using namespace std;

void writeOutput(const string& path, const AllocationResult& result,
                 const vector<Web>& webs, const AlgorithmConfig& config);

#endif //DAPROJECT2_WRITTER_H

#include "Parser.h"
#include "AlgorithmConfig.h"
#include "AllocationResult.h"
#include "RegisterAlloc.h"
#include "Writter.h"
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0]
             << " <ranges_file> <registers_file> <output_file>\n";
        return 1;
    }

    const string rangesFile = argv[1];
    const string configFile = argv[2];
    const string outputFile = argv[3];

    vector<Web> webs;
    Graph<int>  ig;
    AlgorithmConfig config;

    try {
        parseInput(rangesFile, configFile, webs, ig, config);
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }

    AllocationResult result;

    switch (config.type) {
        case AlgorithmType::BASIC:
            result = basicAllocation(webs, ig, config);
            break;
        case AlgorithmType::SPILLING:
            result = spillingAllocation(webs, ig, config);
            break;
        case AlgorithmType::SPLITTING:
            result = splittingAllocation(webs, ig, config);
            break;
        default:
            cerr << "ERROR: Algorithm not implemented yet\n";
            return 1;
    }

    writeOutput(outputFile, result, config);

    if (!result.feasible) {
        cerr << "Register allocation infeasible with " << config.numRegisters << " registers\n";
        return 1;
    }


    return 0;
}
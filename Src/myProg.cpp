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

    const string rangesFile = string("./Data/ranges/") + argv[1];
    const string configFile = string("./Data/registers/") + argv[2];

    vector<Web> webs;
    Graph<int>  ig;
    AlgorithmConfig config;

    try {
        parseInput(rangesFile, configFile, webs, ig, config);
    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }

    string outputFile;
    if (config.type == AlgorithmType::BASIC) outputFile = string("./Data/output/basic/") + argv[3];
    else if (config.type == AlgorithmType::SPILLING) outputFile = string("./Data/output/spilling/") + argv[3];
    else if (config.type == AlgorithmType::SPLITTING) outputFile = string("./Data/output/splitting/") + argv[3];
    else outputFile = string("./Data/output/free/") + argv[3];

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
        case AlgorithmType::FREE:
            result = freeAllocation(webs, ig, config);
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
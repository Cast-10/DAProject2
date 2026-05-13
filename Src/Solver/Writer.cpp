//
// Created by ze on 13/05/26.
//

#include "Writter.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

void writeOutput(const string& path, const AllocationResult& result,
                        const vector<Web>& webs, const AlgorithmConfig& config) {
    ofstream out(path);
    if (!out.is_open()) {
        cerr << "ERROR: Cannot open output file: " << path << "\n";
        return;
    }

    out << "# Webs constructed with respective program points sorted in ascending order\n";
    out << "webs: " << webs.size() << "\n";
    for (const auto& web : webs)
        out << "web" << web.id << ": " << web.pointsString() << "\n";

    out << "# Registers available: " << config.numRegisters << "\n"
        << "# Registers used, followed by assignment webs using " << config.typeName() << " algorithm" << "\n";

    if (!result.feasible) {
        out << "Register allocation infeasible with "
            << config.numRegisters << " registers\n";
        return;
    }

    for (int r = 0; r < result.registersUsed; r++) {
        out << "r" << r << " -> ";
        bool first = true;
        for (const auto& web : webs) {
            if (result.assignment[web.id] == r) {
                if (!first) out << ", ";
                out << "web" << web.id;
                first = false;
            }
        }
        out << "\n";
    }

    out << "# Total registers used: " << result.registersUsed << "\n";
}
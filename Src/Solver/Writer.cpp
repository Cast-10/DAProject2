//
// Created by ze on 13/05/26.
//

#include "Writer.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

void writeOutput(const string& path, const AllocationResult& result,
                        const AlgorithmConfig& config) {
    ofstream out(path);
    if (!out.is_open()) {
        cerr << "ERROR: Cannot open output file: " << path << "\n";
        return;
    }

    const auto& webs = result.webs;

    out << "# Webs constructed with respective program points sorted in ascending order\n";
    out << "webs: " << webs.size() << "\n";
    for (const auto& web : webs)
        out << web.webName() << ": " << web.pointsString() << "\n";

    out << "# Registers available: " << config.numRegisters << "\n";
    if (!result.feasible) {
        out << "Register allocation infeasible with "
            << config.numRegisters << " registers\n";
        return;
    }
    out << "# Registers used, followed by assignment webs using " << config.typeName() << " algorithm\n";


    bool hasSpills = false;
    for (const auto& web : webs)
        if (result.assignment[web.id] < 0) { hasSpills = true; break; }

    if (hasSpills)
        for (const auto& web : webs)
            if (result.assignment[web.id] < 0)
                out << "# " << web.webName() << " was spilled to memory\n";

    for (int r = 0; r < result.registersUsed; r++) {
        out << "r" << r << " -> ";
        bool first = true;
        for (const auto& web : webs) {
            if (result.assignment[web.id] == r) {
                if (!first) out << ", ";
                out << web.webName();
                first = false;
            }
        }
        out << "\n";
    }

    if (hasSpills) {
        out << "M -> ";
        bool first = true;
        for (const auto& web : webs) {
            if (result.assignment[web.id] < 0) {
                if (!first) out << ", ";
                out << web.webName();
                first = false;
            }
        }
        out << "\n";
    }

    out << "# Total registers used: " << result.registersUsed << "\n";
}
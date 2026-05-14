#ifndef PARSER_H
#define PARSER_H

#include "Graph.h"
#include "Point.h"
#include "AlgorithmConfig.h"
#include "../Solver/Build.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

using namespace std;


// Remove whitespace from both ends of a string

static string trim(const string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end   = s.find_last_not_of(" \t\r\n");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}


// Parse a single token, e.g. "7+", "10-", "8"

static Point parsePoint(const string &token) {
    string t = trim(token);
    if (t.empty()) throw runtime_error("Empty program point token");

    bool isDef     = false;
    bool isLastUse = false;

    if (t.back() == '+') { isDef     = true; t.pop_back(); }
    if (t.back() == '-') { isLastUse = true; t.pop_back(); }

    int lineNum = stoi(t); // throws if not a number
    char annot = isDef ? '+' : (isLastUse ? '-' : '\0');
    return Point(lineNum, annot);
}


// Parse one line of the live ranges file into a LiveRange.
// Format: "varName: p1,p2,p3,..."
// Returns false if the line should be skipped (empty / comment).

static bool parseLiveRangeLine(const string &line, LiveRange &lr) {
    string t = trim(line);
    if (t.empty() || t[0] == '#') return false;

    size_t col = t.find(':');
    if (col == string::npos)
        throw runtime_error("Bad live range line (no ':'): " + line);

    lr.varName = trim(t.substr(0, col));

    string pointsStr = t.substr(col + 1);
    stringstream ss(pointsStr);
    string token;
    while (getline(ss, token, ',')) {
        token = trim(token);
        if (!token.empty())
            lr.points.push_back(parsePoint(token));
    }

    // Sort by line number
    sort(lr.points.begin(), lr.points.end());

    if (lr.points.empty())
        throw runtime_error("Live range has no points for var: " + lr.varName);

    return true;
}

// Parse the entire live ranges file.
// Returns a vector of all LiveRange objects read.

static vector<LiveRange> parseLiveRangesFile(const string &filename) {
    ifstream file(filename);
    if (!file.is_open())
        throw runtime_error("Cannot open live ranges file: " + filename);

    vector<LiveRange> ranges;
    string line;
    int lineNo = 0;

    while (getline(file, line)) {
        lineNo++;
        LiveRange lr;
        try {
            if (parseLiveRangeLine(line, lr))
                ranges.push_back(lr);
        } catch (exception &e) {
            cerr << "Warning: skipping malformed line " << lineNo
                 << " in " << filename << ": " << e.what() << endl;
        }
    }

    return ranges;
}


// Parse the registers/config file.
// Format:
//   # comment
//   registers: N
//   algorithm: basic          (or spilling,2 / splitting,2 / free)

static AlgorithmConfig parseConfigFile(const string &filename) {
    ifstream file(filename);
    if (!file.is_open())
        throw runtime_error("Cannot open config file: " + filename);

    AlgorithmConfig cfg;
    string line;

    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t colon = line.find(':');
        if (colon == string::npos) continue;

        string key = trim(line.substr(0, colon));
        string val = trim(line.substr(colon + 1));

        if (key == "registers") {
            cfg.numRegisters = stoi(val);
        } else if (key == "algorithm") {
            // val could be "basic", "free", "spilling, 2", "splitting, 2"
            size_t comma = val.find(',');
            string algName;
            if (comma != string::npos) {
                algName = trim(val.substr(0, comma));
                cfg.param = stoi(trim(val.substr(comma + 1)));
            } else {
                algName = trim(val);
            }
            if (algName == "basic")     cfg.type = AlgorithmType::BASIC;
            if (algName == "spilling")  cfg.type = AlgorithmType::SPILLING;
            if (algName == "splitting") cfg.type = AlgorithmType::SPLITTING;
            if (algName == "free")      cfg.type = AlgorithmType::FREE;
        }
    }

    if (cfg.numRegisters <= 0)
        throw runtime_error("Invalid or missing 'registers' value in config file");

    return cfg;
}


// Main entry point: parse both files and build everything.
// Fills: webs, interferenceGraph, config

static void parseInput(const string &rangesFile,
                        const string &configFile,
                        vector<Web>  &webs,
                        Graph<int>   &interferenceGraph,
                        AlgorithmConfig &config) {

    // 1. Parse live ranges
    vector<LiveRange> ranges = parseLiveRangesFile(rangesFile);
    if (ranges.empty())
        throw runtime_error("No live ranges found in " + rangesFile);

    // 2. Build webs (merge overlapping live ranges)
    webs = buildWebs(ranges);

    // 3. Build interference graph
    interferenceGraph = buildInterferenceGraph(webs);

    // 4. Parse config
    config = parseConfigFile(configFile);


}

#endif // PARSER_H
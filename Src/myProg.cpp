#include <iostream>
#include <string>
#include <vector>
#include "DataStructures/Graph.h"
#include "RegisterAlloc.h"
#include "Parser.h"

using namespace std;

// ─── Forward declarations ───────────────────
void runInteractive(vector<Web> &webs, Graph<int> &ig, AllocConfig &cfg);
void runBatch(const string &rangesFile, const string &configFile, const string &outputFile);

// ─────────────────────────────────────────────
int main(int argc, char *argv[]) {

    // Batch mode: myProg -b ranges.txt registers.txt allocation.txt
    if (argc == 5 && string(argv[1]) == "-b") {
        string rangesFile = argv[2];
        string configFile = argv[3];
        string outputFile = argv[4];
        runBatch(rangesFile, configFile, outputFile);
        return 0;
    }

    // Interactive mode
    vector<Web>   webs;
    Graph<int>    ig;
    AllocConfig   cfg;

    runInteractive(webs, ig, cfg);
    return 0;
}

// ─────────────────────────────────────────────
void runBatch(const string &rangesFile, const string &configFile, const string &outputFile) {
    try {
        vector<Web>  webs;
        Graph<int>   ig;
        AllocConfig  cfg;

        parseInput(rangesFile, configFile, webs, ig, cfg);

        // TODO: run allocation algorithm
        // TODO: write output file

        cout << "Batch done. Output -> " << outputFile << "\n";
    } catch (exception &e) {
        cerr << "Error: " << e.what() << "\n";
    }
}

// ─────────────────────────────────────────────
void runInteractive(vector<Web> &webs, Graph<int> &ig, AllocConfig &cfg) {
    string rangesFile, configFile;
    bool loaded = false;

    while (true) {
        cout << "\n========== Register Allocator ==========\n";
        cout << " 1. Load input files\n";
        cout << " 2. Show webs\n";
        cout << " 3. Show interference graph\n";
        cout << " 4. Run allocation (basic)\n";
        cout << " 0. Exit\n";
        cout << "Choice: ";

        int choice;
        if (!(cin >> choice)) { cin.clear(); cin.ignore(1000,'\n'); continue; }

        switch (choice) {
            case 1: {
                cout << "Live ranges file: "; cin >> rangesFile;
                cout << "Config file: ";      cin >> configFile;
                try {
                    parseInput(rangesFile, configFile, webs, ig, cfg);
                    loaded = true;
                    cout << "Loaded successfully.\n";
                } catch (exception &e) {
                    cerr << "Error loading: " << e.what() << "\n";
                }
                break;
            }
            case 2: {
                if (!loaded) { cout << "Load files first.\n"; break; }
                cout << "\n--- Webs ---\n";
                for (auto &w : webs) cout << "  " << w.toString() << "\n";
                break;
            }
            case 3: {
                if (!loaded) { cout << "Load files first.\n"; break; }
                cout << "\n--- Interference Graph ---\n";
                for (auto *v : ig.getVertexSet()) {
                    cout << "  web" << v->getInfo() << ": ";
                    for (auto &e : v->getAdj())
                        cout << "web" << e.getDest()->getInfo() << " ";
                    cout << "\n";
                }
                break;
            }
            case 4: {
                if (!loaded) { cout << "Load files first.\n"; break; }
                // TODO: call allocation algorithm
                cout << "Algorithm not yet implemented.\n";
                break;
            }
            case 0:
                cout << "Bye.\n";
                return;
            default:
                cout << "Invalid option.\n";
        }
    }
}
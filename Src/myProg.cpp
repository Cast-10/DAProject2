#include "Parser.h"
#include "AlgorithmConfig.h"
#include "AllocationResult.h"
#include "RegisterAlloc.h"
#include "Writer.h"
#include <iostream>
#include <vector>
//color definitions
#define PINK   "\033[35m"
#define CYAN   "\033[36m"
#define YELLOW "\033[33m"
#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define BLUE   "\033[34m"
#define RESET  "\033[0m"
#define BOLD   "\033[1m"
#define BLINK  "\033[5m"
using namespace std;

static int menu() {
    int choice;
    do {
        vector<Web> webs;
        Graph<int> ig;
        AlgorithmConfig config;
        cout<<RED;
        cout << "\n⫘⫘⫘⫘⫘⫘ "<<YELLOW<<"REGISTER ALLOCATION MENU"<<RED<<" ⫘⫘⫘⫘⫘⫘\n";
        cout<<CYAN;
        cout << "1. BASIC allocation\n";
        cout << "2. SPILLING allocation\n";
        cout << "3. SPLITTING allocation\n";
        cout << "4. FREE allocation\n";
        cout << "0. Exit\n"<<RED;
        cout << "⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘\n";
        cout<<YELLOW;
        cout << "Choose an algorithm: ";
        cin >> choice;

        switch (choice) {
            case 0:
                cout << "Exiting...\n";
                return 0;
            case 1:
                config.type = AlgorithmType::BASIC;
                break;
            case 2:
                config.type = AlgorithmType::SPILLING;
                break;
            case 3:
                config.type = AlgorithmType::SPLITTING;
                break;
            case 4:
                config.type = AlgorithmType::FREE;
                break;
            default:
                cerr << "ERROR: Invalid option!\n";
                continue;
        }

        string rangeIndex, regIndex, outputName;

        cout << "\nChoose ranges index (ranges[i].txt): ";
        cin >> rangeIndex;
        cout << "Choose registers index ([algorithm]_regs[i].txt): ";
        cin >> regIndex;
        cout << "Choose your output file: ";
        cin >> outputName;
        string rangesFile =
            "../Data/ranges/ranges" + rangeIndex + ".txt";

        string algorithmName;

        switch (config.type) {
        case AlgorithmType::BASIC:
            algorithmName = "basic";
            break;
        case AlgorithmType::SPILLING:
            algorithmName = "spilling";
            break;
        case AlgorithmType::SPLITTING:
            algorithmName = "splitting";
            break;
        case AlgorithmType::FREE:
            algorithmName = "free";
            break;
        }
        string configFile = "../Data/registers/" + algorithmName + "_regs" + regIndex + ".txt";

        try {
            parseInput(rangesFile, configFile, webs, ig, config);
        } catch (const exception& e) {
            cerr << "ERROR: " << e.what() << "\n";
            continue;
        }

        string outputFile;

        switch (config.type) {
            case AlgorithmType::BASIC:
                outputFile = "../Data/output/basic/" + outputName;
                break;
            case AlgorithmType::SPILLING:
                outputFile = "../Data/output/spilling/" + outputName;
                break;
            case AlgorithmType::SPLITTING:
                outputFile = "../Data/output/splitting/" + outputName;
                break;
            case AlgorithmType::FREE:
                outputFile = "../Data/output/free/" + outputName;
                break;
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
            case AlgorithmType::FREE:
                result = freeAllocation(webs, ig, config);
                break;
            default:
                cerr << "ERROR: Algorithm not implemented yet\n";
                continue;
        }

        writeOutput(outputFile, result, config);

        int choice2;
        bool showWebs = false;
        bool showRegs = false;
        while (!(showWebs && showRegs))
        {
            cout << "\n";
            cout <<RED<< "⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘\n";
            cout <<CYAN<< "1. Webs used\n";
            cout << "2. Registers used\n";
            cout << "0. Skip\n" <<RESET;
            cout <<RED<< "⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘\n";
            cout<<YELLOW;
            cout << "Choose an option: ";
            cin >> choice2;
            switch (choice2) {
            case 1:
                if (!showWebs)
                {
                    cout<<PINK<<"\n⫘⫘⫘⫘⫘⫘ "<<YELLOW<<"WEBS USED"<<PINK<<" ⫘⫘⫘⫘⫘⫘\n"<<BLUE;
                    for (const auto& w : webs) {
                        cout << "Web " << w.id
                             << " (" << w.webName() << ") : "<< w.pointsString()<< "\n";}
                    cout <<PINK<< "⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘\n";
                    showWebs = true;
                }
                break;
            case 2:
                if (!showRegs) {
                    cout<<PINK<<"\n⫘⫘⫘⫘⫘⫘ "<<YELLOW<<"REGISTERS USED"<<PINK<<" ⫘⫘⫘⫘⫘⫘\n"<<BLUE;
                    for (size_t i = 0; i < result.assignment.size(); i++) {
                        cout << "Web " << i<< " -> " << result.registerName(i)<< "\n";}
                    cout <<PINK<< "⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘\n";
                    showRegs = true;}
                break;
            case 0:
                showWebs = true;
                showRegs = true;
                break;
            default:
                cout << "Invalid option\n";
            }
        }


        cout<<PINK;
        cout << "\n⫘⫘⫘⫘⫘⫘"<<YELLOW<<" RESULT"<<PINK<<" ⫘⫘⫘⫘⫘⫘\n"<<BLUE;
        switch (config.type) {
            case AlgorithmType::BASIC:
                cout << "Algorithm: BASIC\n";
                break;
            case AlgorithmType::SPILLING:
                cout << "Algorithm: SPILLING\n";
                break;
            case AlgorithmType::SPLITTING:
                cout << "Algorithm: SPLITTING\n";
                break;
            case AlgorithmType::FREE:
                cout << "Algorithm: FREE\n";
                break;
        }

        cout << "Registers: "
             << config.numRegisters << "\n";

        if (result.feasible)
            cout << "Allocation feasible with "<< config.numRegisters << " registers\n";
        else
            cout << "Allocation infeasible with "<< config.numRegisters << " registers\n";
        cout << "Output written to: "
             << outputFile << "\n";
        cout<<PINK;
        cout << "⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘\n";

    } while (true);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        if (menu() != 0) return 1;
        return 0;
    }
    if (argc < 4) {
        cerr << "Usage: " << argv[0]
             << " <ranges_file> <registers_file> <output_file>\n";
        return 1;
    }

    const string rangesFile = string("../Data/ranges/") + argv[1];
    const string configFile = string("../Data/registers/") + argv[2];

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
    if (config.type == AlgorithmType::BASIC) outputFile = string("../Data/output/basic/") + argv[3];
    else if (config.type == AlgorithmType::SPILLING) outputFile = string("../Data/output/spilling/") + argv[3];
    else if (config.type == AlgorithmType::SPLITTING) outputFile = string("../Data/output/splitting/") + argv[3];
    else outputFile = string("../Data/output/free/") + argv[3];

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
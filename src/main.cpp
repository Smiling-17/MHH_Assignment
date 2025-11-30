/*
 * main.cpp - Petri Net Solver CLI
 * Integrates all modules: Parser, Explicit, BDD, ILP
 * Usage: ./petri_solver --input <file.pnml> --mode <all|explicit|bdd> [--optimize]
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <cstdlib>

#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(dir) _mkdir(dir)
#else
    #include <sys/stat.h>
    #define MKDIR(dir) mkdir(dir, 0755)
#endif

#include "utils.h"
#include "parser.h"
#include "reachability.h"
#include "bdd.h"

#ifdef HAS_GLPK
    #include "ilp.h"
#endif

using namespace std;

void createDirectory(const string& path) {
    string dir = path;
    if (!dir.empty() && (dir.back() == '/' || dir.back() == '\\')) {
        dir.pop_back();
    }
    if (!dir.empty()) {
        MKDIR(dir.c_str());
    }
}

void printUsage() {
    cout << "Usage: ./petri_solver --input <file.pnml> [options]\n";
    cout << "Options:\n";
    cout << "  --input <file>     : Path to input PNML file (Required)\n";
    cout << "  --mode <mode>      : 'explicit', 'bdd', or 'all' (Default: all)\n";
    cout << "  --optimize         : Enable ILP Optimization (Task 5)\n";
    cout << "  --outdir <path>    : Directory to save results (Default: output/)\n";
    cout << "  --help             : Show this help message\n";
    cout << "Example:\n";
    cout << "  ./petri_solver --input data/simple_test.pnml --mode bdd --optimize\n";
}

int main(int argc, char* argv[]) {
    // Parse CLI arguments
    string inputFile;
    string mode = "all";
    string outDir = "output/";
    bool doOptimize = false;

    if (argc < 2) {
        printUsage();
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = argv[++i];
        } else if (strcmp(argv[i], "--outdir") == 0 && i + 1 < argc) {
            outDir = argv[++i];
        } else if (strcmp(argv[i], "--optimize") == 0) {
            doOptimize = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            printUsage();
            return 0;
        }
    }

    if (inputFile.empty()) {
        cerr << "[ERROR] Missing input file! Use --input.\n";
        return 1;
    }

    createDirectory(outDir);

    // Prepare CSV output
    string csvPath = outDir + "result.csv";
    bool fileExists = false;
    ifstream checkFile(csvPath);
    if (checkFile.good()) fileExists = true;
    checkFile.close();

    ofstream csvFile(csvPath, ios::app);
    if (!csvFile.is_open()) {
        cerr << "[ERROR] Cannot open " << csvPath << "\n";
        return 1;
    }

    if (!fileExists)
        csvFile << "Model,Method,States,TimeSec,MemMB,Deadlock,OptObj,OptMarking\n";

    string modelName = inputFile.substr(inputFile.find_last_of("/\\") + 1);

    try {
        // Task 1: Parse PNML
        cout << "[INFO] Parsing PNML: " << inputFile << "..." << endl;
        Model model = parsePNML(inputFile, true, outDir + "petri_net.dot");
        cout << "[INFO] Parsed successfully. Places: " << model.places.size() 
             << ", Transitions: " << model.transitions.size() << endl;

        // Task 2: Explicit Reachability
        if (mode == "explicit" || mode == "all") {
            cout << "[INFO] Task 2: Running Explicit Reachability (BFS/DFS)..." << endl;
            ReachOptions reachOpts;
            ReachResult res = explicitReach(model, reachOpts);
            cout << "       -> States: " << res.states << ", Time: " << res.timeSec << "s" << endl;
            csvFile << modelName << ",Explicit," << res.states << "," 
                    << res.timeSec << "," << res.memMB << ",N/A,N/A,N/A\n";
        }

        // Task 3: Symbolic Reachability (BDD)
        BddResult bddRes;
        if (mode == "bdd" || mode == "all") {
            cout << "[INFO] Task 3: Running Symbolic Reachability (BDD)..." << endl;
            BddOptions bddOpts;
            bddRes = bddReach(model, bddOpts);
            cout << "       -> States: " << bddRes.states << ", Nodes: " << bddRes.nodeCount 
                 << ", Time: " << bddRes.timeSec << "s" << endl;
            csvFile << modelName << ",BDD," << bddRes.states << "," 
                    << bddRes.timeSec << "," << bddRes.memMB << ",";
        }


        // Task 4 & 5: ILP-based analysis (requires GLPK)
#ifdef HAS_GLPK
        if (mode == "bdd" || mode == "all") {
            // Task 4: Deadlock Detection
            cout << "[INFO] Task 4: Detecting Deadlock (ILP + BDD)..." << endl;
            IlpOptions ilpOpts;
            ilpOpts.mode = IlpMode::DEADLOCK;
            IlpResult deadlockRes = solveILP(model, bddRes, ilpOpts);
            
            ofstream dlFile(outDir + "deadlock.txt");
            if (deadlockRes.hasDeadlock && deadlockRes.isReachable) {
                cout << "       [FOUND] Deadlock at: " << toString(deadlockRes.deadlockMarking) << endl;
                csvFile << "Yes,";
                dlFile << "Deadlock: " << toString(deadlockRes.deadlockMarking) << endl;
            } else {
                cout << "       [NONE] No reachable deadlock found." << endl;
                csvFile << "No,";
                dlFile << "None" << endl;
            }
            dlFile.close();

            // Task 5: Optimization
            if (doOptimize) {
                cout << "[INFO] Task 5: Optimizing Objective (Maximize c^T M)..." << endl;
                IlpOptions optOpts;
                optOpts.mode = IlpMode::OPTIMIZATION;
                optOpts.weights.assign(model.places.size(), 1);
                IlpResult optRes = solveILP(model, bddRes, optOpts);
                
                ofstream optFile(outDir + "optimum.txt");
                if (optRes.isReachable) {
                    cout << "       -> Max Value: " << optRes.optObj << endl;
                    csvFile << optRes.optObj << ",\"" << toString(optRes.optMarking) << "\"\n";
                    optFile << "Max: " << optRes.optObj << ", Marking: " << toString(optRes.optMarking) << "\n";
                } else {
                    csvFile << "N/A,N/A\n";
                    optFile << "None\n";
                }
                optFile.close();
            } else {
                csvFile << "N/A,N/A\n";
            }
        }
#else
        if (mode == "bdd" || mode == "all") {
            cout << "[WARN] GLPK not available - Task 4 & 5 skipped" << endl;
            csvFile << "N/A,N/A,N/A\n";
        }
#endif

        if (bddRes.internalState) bdd_cleanup(bddRes);

    } catch (const exception& e) {
        cerr << "\n[FATAL ERROR] Exception occurred: " << e.what() << endl;
        csvFile.close();
        return 1;
    }

    cout << "[INFO] All tasks completed. Results saved to '" << outDir << "'." << endl;
    csvFile.close();
    
    return 0;
}
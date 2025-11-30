#ifndef ILP_H
#define ILP_H

/*
 * ilp.h - ILP-based analysis using GLPK (Task 4 & 5)
 * Task 4: Deadlock detection - find reachable marking with no enabled transitions
 * Task 5: Optimization - maximize c^T * M over reachable markings
 */

#include "utils.h"
#include <vector>

enum class IlpMode {
    DEADLOCK,      // Task 4: Find deadlock
    OPTIMIZATION   // Task 5: Maximize objective
};

struct IlpOptions {
    IlpMode mode = IlpMode::DEADLOCK;
    std::vector<int> weights;  // Coefficients c for optimization
    bool verbose = false;
    int maxCuts = 10000;       // Max cutting-plane iterations
};

// Solve ILP with BDD reachability checking
IlpResult solveILP(const Model& model,
                   const BddResult& bddResult,
                   const IlpOptions& options);

#endif
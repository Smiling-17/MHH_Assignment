#ifndef ILP_H
#define ILP_H

#include "utils.h"
#include <vector>

// ILP modes
enum class IlpMode {
    DEADLOCK,
    OPTIMIZATION
};

// Options for ILP
struct IlpOptions {
    IlpMode mode = IlpMode::DEADLOCK;
    std::vector<int> weights;        // For optimization
    bool verbose = false;            // Enable verbose logging
    int maxCuts = 10000;             // Maximum cutting planes for optimization
};

// Main entrypoint used by the whole system
IlpResult solveILP(const Model& model,
                  const BddResult& bddResult,
                  const IlpOptions& options);

#endif // ILP_H
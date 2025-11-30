#ifndef REACHABILITY_H
#define REACHABILITY_H

/*
 * reachability.h - Explicit state-space exploration (Task 2)
 * Computes all reachable markings using BFS or DFS
 */

#include "utils.h"

struct ReachOptions {
    bool useBFS = true;  // true = BFS, false = DFS
};

class ExplicitReachability {
public:
    ExplicitReachability(const Model &model);
    ReachResult computeBFS();  // Breadth-first search
    ReachResult computeDFS();  // Depth-first search
private:
    const Model &petri_net;
};

// Main entry point
ReachResult explicitReach(const Model &model, const ReachOptions &opts);

#endif

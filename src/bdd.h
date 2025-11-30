#ifndef MY_ASSIGNMENT_BDD_H  
#define MY_ASSIGNMENT_BDD_H

/*
 * bdd.h - Symbolic reachability using BDDs (Task 3)
 * Uses BuDDy library to compute reachable states symbolically
 */

#include "utils.h"

struct BddOptions {
    int maxIters = 1000;
    bool useGC = true;
};

// Compute reachable states using BDD fixpoint
BddResult bddReach(const Model& net, const BddOptions& opts);

// Check if marking M is in the reachable set (used by ILP)
bool bdd_check_reachable(const BddResult& bddResult, const Marking& M, int numPlaces);

// Free BDD resources
void bdd_cleanup(BddResult& bddResult);

#endif
/*
 * bdd.cpp - Symbolic reachability using BuDDy BDD library (Task 3)
 * Encodes markings and transition relation as BDDs, computes fixpoint
 */

#include "bdd.h"
#include "utils.h"
#include "buddy/bdd.h"
#include <iostream>
#include <chrono>

// Convert marking to BDD (conjunction of variable assignments)
bdd markingToBdd(const Marking& m, int numPlaces) {
    bdd result = bdd_true();
    for (int i = 0; i < numPlaces; ++i) {
        int varIdx = i * 2; // Biến chẵn: trạng thái hiện tại (x)

        // Logic 1-safe: 
        // Nếu có token (1) -> AND với biến x
        // Nếu không có (0) -> AND với phủ định !x
        if (m[i] > 0) {
            result &= bdd_ithvar(varIdx);
        }
        else {
            result &= bdd_nithvar(varIdx);
        }
    }
    return result;
}

// Main symbolic reachability algorithm
BddResult bddReach(const Model& net, const BddOptions& opts) {
    auto start_time = std::chrono::high_resolution_clock::now();

    BddResult res;
    int numPlaces = net.places.size();
    int numVars = numPlaces * 2;  // Current (even) + Next (odd) vars

    // Initialize BuDDy
    if (!bdd_isrunning()) {
        bdd_init(1000000, 100000);
        bdd_setvarnum(numVars);
    }

    // Encode initial marking M0
    bdd M0_bdd = markingToBdd(net.M0, numPlaces);

    // Build transition relation: TR = OR of all transitions
    bdd TR = bdd_false();

    // Variable renaming pairs for next->current substitution
    bddPair* pairs = bdd_newpair();
    for (int i = 0; i < numPlaces; ++i) {
        // Map biến lẻ (Next - 2i+1) về biến chẵn (Curr - 2i)
        bdd_setpair(pairs, i * 2 + 1, i * 2);
    }

    // Encode each transition
    for (size_t t = 0; t < net.transitions.size(); ++t) {
        bdd trans_t = bdd_true();
        for (int p = 0; p < numPlaces; ++p) {
            int curr = p * 2;      // Current state var
            int next = p * 2 + 1;  // Next state var
            int preVal = net.Pre[p][t];
            int postVal = net.Post[p][t];

            // Enable condition: need token if Pre=1
            if (preVal == 1) trans_t &= bdd_ithvar(curr);

            // Next state: token produced, consumed, or unchanged
            if (postVal == 1)
                trans_t &= bdd_ithvar(next);
            else if (preVal == 1 && postVal == 0)
                trans_t &= bdd_nithvar(next);
            else
                trans_t &= bdd_apply(bdd_ithvar(curr), bdd_ithvar(next), bddop_biimp);
        }
        TR |= trans_t;
    }

    // Fixpoint computation: Reached = Reached ∪ Image(New) until stable
    bdd Reached = M0_bdd;
    bdd New = M0_bdd;
    bdd currentVarSet = bdd_true();
    for (int i = 0; i < numPlaces; ++i)
        currentVarSet &= bdd_ithvar(i * 2);

    int loopCount = 0;
    while (true) {
        if (++loopCount > opts.maxIters) break;

        // Image computation: ∃x. (New(x) ∧ TR(x,x'))
        bdd next_prime = bdd_relprod(New, TR, currentVarSet);
        bdd next_state = bdd_replace(next_prime, pairs);  // Rename x' -> x
        bdd new_diff = next_state - Reached;

        if (new_diff == bdd_false()) break;  // Fixpoint reached

        Reached |= new_diff;
        New = new_diff;
    }

    // Collect results
    res.states = (size_t)bdd_satcountset(Reached, currentVarSet);
    res.nodeCount = bdd_getnodenum();
    res.iters = loopCount;
    res.timeSec = std::chrono::duration<double>(
        std::chrono::high_resolution_clock::now() - start_time).count();

    // Store BDD for later reachability checks
    int reachedID = Reached.id();
    bdd_addref(reachedID);
    res.internalState = new int(reachedID);

    bdd_freepair(pairs);
    return res;
}

// Check if marking M is in the Reached set (M ∧ Reached ≠ false)
bool bdd_check_reachable(const BddResult& bddResult, const Marking& M, int numPlaces) {
    if (!bddResult.internalState || !bdd_isrunning()) return false;
    
    BDD ReachedBDD = *static_cast<int*>(bddResult.internalState);
    BDD markingBDD = markingToBdd(M, numPlaces).id();
    BDD resultBDD = bdd_addref(bdd_and(markingBDD, ReachedBDD));
    
    bool isReachable = (resultBDD != 0);
    bdd_delref(resultBDD);
    return isReachable;
}

// Free BDD resources
void bdd_cleanup(BddResult& bddResult) {
    if (bddResult.internalState) {
        int* storedID = static_cast<int*>(bddResult.internalState);
        if (bdd_isrunning()) bdd_delref(*storedID);
        delete storedID;
        bddResult.internalState = nullptr;
    }
}
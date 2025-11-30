#ifndef BDD_MOCK_H
#define BDD_MOCK_H

#include "utils.h"
#include <unordered_set>
#include <vector>
#include <queue>

// ---------------- Mock BDD Manager ----------------
struct MockBddManager {
    std::unordered_set<Marking, MarkingHash> reachableStates;

    inline void addReachableState(const Marking& M) {
        reachableStates.insert(M);
    }

    inline bool isReachable(const Marking& M) const {
        return reachableStates.find(M) != reachableStates.end();
    }
};

// --------------- Build reachable states (BFS) ---------------
inline BddResult createMockBddResult(const Model& model) {
    BddResult result;

    MockBddManager* mockMgr = new MockBddManager();

    std::unordered_set<Marking, MarkingHash> visited;
    std::queue<Marking> q;

    q.push(model.M0);
    visited.insert(model.M0);
    mockMgr->addReachableState(model.M0);

    while (!q.empty()) {
        Marking cur = q.front();
        q.pop();

        for (size_t t = 0; t < model.transitions.size(); ++t) {
            if (isEnabled(model, cur, (int)t)) {
                Marking nxt = fire(model, cur, (int)t);
                if (visited.find(nxt) == visited.end()) {
                    visited.insert(nxt);
                    mockMgr->addReachableState(nxt);
                    q.push(nxt);
                }
            }
        }
    }

    result.states = visited.size();
    result.internalState = mockMgr;
    result.timeSec = 0.001;
    result.memMB = 1.0;
    result.nodeCount = visited.size();
    result.iters = 1;

    return result;
}

// --------------- BDD reachability test ---------------
inline bool bdd_check_marking(void* bddState, const Marking& M) {
    if (!bddState) return false;
    MockBddManager* mgr = static_cast<MockBddManager*>(bddState);
    return mgr->isReachable(M);
}

// --------------- Cleanup ---------------
inline void bdd_cleanup_mock(BddResult& result) {
    if (result.internalState) {
        delete static_cast<MockBddManager*>(result.internalState);
        result.internalState = nullptr;
    }
}

#endif

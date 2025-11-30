#ifndef REACHABILITY_H
#define REACHABILITY_H

#include "utils.h"

struct ReachOptions {
    bool useBFS = true; // true = BFS , false = DFS
};

class ExplicitReachability {
    public:
        ExplicitReachability(const Model &model);

        ReachResult computeBFS();

        ReachResult computeDFS();

    private:
        const Model &petri_net;
};

ReachResult explicitReach(const Model &model, const ReachOptions &opts);

#endif

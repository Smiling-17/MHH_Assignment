// src/ilp.cpp (FIXED VERSION with proper BDD check + min-cardinality deadlock objective)
// Author: Ngô Minh Khôi
// Description: ILP-based Deadlock Detection & Optimization using GLPK + BDD checks

#include "ilp.h"
#include "utils.h"
#include "bdd.h"  // Sử dụng BDD thật từ Task 3 (thay vì mock)
#include <glpk.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
#include <string>
#include <limits>

using namespace std;

// -------------------- Internal helpers --------------------
static bool isDeadlock(const Model& model, const Marking& M) {
    for (size_t t = 0; t < model.transitions.size(); ++t)
        if (isEnabled(model, M, (int)t)) return false;
    return true;
}

// Gọi hàm BDD thật để kiểm tra reachability (từ bdd.h)
// numPlaces cần được truyền vào từ model
static bool isReachableViaBDD(const BddResult& bddResult, const Marking& M, int numPlaces) {
    if (bddResult.internalState == nullptr) {
        cerr << "[WARN] BDD internal state is null, assuming reachable for testing\n";
        return true;  // Fallback cho testing
    }
    return bdd_check_reachable(bddResult, M, numPlaces);
}

// -------------------- Deadlock detection (GLPK) --------------------
static IlpResult solveDeadlockILP(const Model& model,
                                  const BddResult& bddResult,
                                  const IlpOptions& options) {
    IlpResult result;
    auto t_start = chrono::high_resolution_clock::now();

    const size_t P = model.places.size();
    const size_t T = model.transitions.size();

    if (P == 0 || T == 0) {
        if (options.verbose) cerr << "[ILP] Empty model (no places or no transitions).\n";
        result.hasDeadlock = false;
        result.isReachable = false;
        result.timeSec = chrono::duration<double>(chrono::high_resolution_clock::now() - t_start).count();
        return result;
    }

    // Quick check: if any transition has totalPre == 0 then always enabled
    for (size_t t = 0; t < T; ++t) {
        int totalPre = 0;
        for (size_t p = 0; p < P; ++p) totalPre += model.Pre[p][t];
        if (totalPre == 0) {
            if (options.verbose)
                cout << "[ILP] Transition " << t << " requires 0 tokens -> always enabled -> no deadlock.\n";
            result.hasDeadlock = false;
            result.isReachable = false;
            result.timeSec = chrono::duration<double>(chrono::high_resolution_clock::now() - t_start).count();
            return result;
        }
    }

    glp_prob* lp = nullptr;
    try {
        lp = glp_create_prob();
        glp_set_prob_name(lp, "DeadlockDetection");

        // **Set objective to MINIMIZE sum_p M_p** so solver prefers minimal-cardinality deadlocks
        glp_set_obj_dir(lp, GLP_MIN);

        // Columns: M[p] as binary (1-safe)
        glp_add_cols(lp, (int)P);
        vector<string> colNames(P);
        for (size_t p = 0; p < P; ++p) {
            colNames[p] = "M_" + model.places[p];
            glp_set_col_name(lp, (int)p + 1, colNames[p].c_str());
            glp_set_col_kind(lp, (int)p + 1, GLP_BV);
            // objective coefficient = 1 (minimize number of tokens)
            glp_set_obj_coef(lp, (int)p + 1, 1.0);
        }

        // Rows: for each transition, require insufficient tokens (no transition enabled)
        glp_add_rows(lp, (int)T);
        for (int t = 0; t < (int)T; ++t) {
            int totalPre = 0;
            for (size_t p = 0; p < P; ++p) totalPre += model.Pre[p][t];

            int nz = 0;
            for (size_t p = 0; p < P; ++p) if (model.Pre[p][t] > 0) ++nz;

            vector<int> ind(nz + 1);
            vector<double> val(nz + 1);
            int idx = 0;
            for (size_t p = 0; p < P; ++p) {
                if (model.Pre[p][t] > 0) {
                    ++idx;
                    ind[idx] = (int)p + 1;
                    val[idx] = (double)model.Pre[p][t];
                }
            }

            glp_set_mat_row(lp, t + 1, nz, ind.data(), val.data());
            glp_set_row_bnds(lp, t + 1, GLP_UP, 0.0, (double)(totalPre - 1));
        }

        // We'll iteratively query GLPK: if candidate is not reachable (via BDD), add a cut to exclude it and repeat
        glp_term_out(GLP_OFF);  // silence GLPK

        int cuts = 0;
        const int maxCuts = options.maxCuts > 0 ? options.maxCuts : 10000;

        while (true) {
            // solve MIP
            glp_simplex(lp, nullptr);
            glp_intopt(lp, nullptr);

            int status = glp_mip_status(lp);
            if (!(status == GLP_OPT || status == GLP_FEAS)) {
                if (options.verbose)
                    cout << "[ILP] No feasible deadlock marking (status=" << status << ")\n";
                break;
            }

            Marking cand(P);
            for (size_t p = 0; p < P; ++p) {
                double v = glp_mip_col_val(lp, (int)p + 1);
                cand[p] = (Token)(v > 0.5 ? 1 : 0);
            }

            if (options.verbose)
                cout << "[ILP] Candidate dead marking: " << toString(cand) << endl;

            bool reachable = isReachableViaBDD(bddResult, cand, (int)P);
            bool isDead = isDeadlock(model, cand);

            if (options.verbose) {
                cout << "[ILP]   -> Reachable? " << (reachable ? "YES" : "NO") << endl;
                cout << "[ILP]   -> Is dead? " << (isDead ? "YES" : "NO") << endl;
            }

            if (reachable && isDead) {
                result.hasDeadlock = true;
                result.isReachable = true;
                result.deadlockMarking = cand;
                break;
            }

            // Exclude this candidate and continue search (exclude exact assignment only)
            int k = 0;
            for (size_t p = 0; p < P; ++p) if (cand[p]) ++k;

            if (k == 0) {
                // exclude zero vector by forcing at least one 1: sum_p M_p >= 1
                vector<int> ind((int)P + 1);
                vector<double> val((int)P + 1);
                for (size_t p = 0; p < P; ++p) {
                    ind[(int)p + 1] = (int)p + 1;
                    val[(int)p + 1] = 1.0;
                }
                glp_add_rows(lp, 1);
                int row = glp_get_num_rows(lp);
                glp_set_mat_row(lp, row, (int)P, ind.data(), val.data());
                glp_set_row_bnds(lp, row, GLP_LO, 1.0, 0.0);
                if (options.verbose) cout << "[ILP] Excluding zero-candidate (require >=1 token)\n";
            } else {
                // Exclude exact assignment c:
                // sum_{p: c_p=1} M_p - sum_{p: c_p=0} M_p <= k - 1
                vector<int> ind((int)P + 1);
                vector<double> val((int)P + 1);
                int idx = 0;
                for (size_t p = 0; p < P; ++p) {
                    ++idx;
                    ind[idx] = (int)p + 1;
                    val[idx] = (cand[p] ? 1.0 : -1.0);
                }
                glp_add_rows(lp, 1);
                int row = glp_get_num_rows(lp);
                glp_set_mat_row(lp, row, (int)P, ind.data(), val.data());
                glp_set_row_bnds(lp, row, GLP_UP, 0.0, (double)(k - 1));
                if (options.verbose) cout << "[ILP] Excluding exact candidate (cut #" << cuts+1 << ")\n";
            }

            ++cuts;
            if (cuts > maxCuts) {
                if (options.verbose) cout << "[ILP] Max cuts (" << maxCuts << ") reached.\n";
                break;
            }
        }

    } catch (const exception& e) {
        cerr << "[ILP][ERROR] Exception: " << e.what() << endl;
    }

    if (lp) {
        glp_delete_prob(lp);
        lp = nullptr;
    }

    result.timeSec = chrono::duration<double>(chrono::high_resolution_clock::now() - t_start).count();
    return result;
}

// -------------------- Optimization (GLPK) --------------------
static IlpResult solveOptimizationILP(const Model& model,
                                     const BddResult& bddResult,
                                     const IlpOptions& options) {
    IlpResult result;
    auto t_start = chrono::high_resolution_clock::now();

    const size_t P = model.places.size();
    if (P == 0) {
        if (options.verbose) cerr << "[ILP] Empty model (no places).\n";
        result.isReachable = false;
        result.timeSec = chrono::duration<double>(chrono::high_resolution_clock::now() - t_start).count();
        return result;
    }

    if (options.weights.size() != P) {
        if (options.verbose) cerr << "[ILP] Weight vector size mismatch.\n";
        result.isReachable = false;
        return result;
    }

    glp_prob* lp = nullptr;
    try {
        lp = glp_create_prob();
        glp_set_prob_name(lp, "MarkingOptimization");
        glp_set_obj_dir(lp, GLP_MAX);

        glp_add_cols(lp, (int)P);
        vector<string> colNames(P);
        for (size_t p = 0; p < P; ++p) {
            colNames[p] = "M_" + model.places[p];
            glp_set_col_name(lp, (int)p + 1, colNames[p].c_str());
            glp_set_col_kind(lp, (int)p + 1, GLP_BV);
            glp_set_obj_coef(lp, (int)p + 1, (double)options.weights[p]);
        }

        // For optimization we don't add deadlock constraints here (seek reachable marking maximizing c^T M)
        const int maxCuts = options.maxCuts > 0 ? options.maxCuts : 10000;
        int cuts = 0;
        bool found = false;
        Marking bestM(P);
        double bestObj = -numeric_limits<double>::infinity();

        glp_term_out(GLP_OFF);  // Tắt output của GLPK

        // Note: We iterate because GLPK might return non-reachable candidate; exclude them progressively
        while (true) {
            glp_simplex(lp, nullptr);
            glp_intopt(lp, nullptr);
            int status = glp_mip_status(lp);

            if (!(status == GLP_OPT || status == GLP_FEAS)) {
                if (options.verbose) 
                    cout << "[ILP] No feasible solutions left (status=" << status << ").\n";
                break;
            }

            Marking cand(P);
            for (size_t p = 0; p < P; ++p) {
                double v = glp_mip_col_val(lp, (int)p + 1);
                cand[p] = (Token)(v > 0.5 ? 1 : 0);
            }
            double obj = glp_mip_obj_val(lp);

            if (options.verbose) 
                cout << "[ILP] Candidate: " << toString(cand) << " (obj=" << obj << ")\n";

            if (isReachableViaBDD(bddResult, cand, (int)P)) {
                found = true;
                bestM = cand;
                bestObj = obj;
                if (options.verbose) 
                    cout << "[ILP] Candidate is reachable. Accepting.\n";
                break;
            }

            // Add cutting plane to exclude this candidate (exact assignment)
            int k = 0;
            for (size_t p = 0; p < P; ++p) if (cand[p]) ++k;
            if (k == 0) {
                if (options.verbose) 
                    cout << "[ILP] Empty candidate not reachable -> stop.\n";
                break;
            }

            // build exact-exclude constraint:
            // sum_{p: c_p=1} M_p - sum_{p: c_p=0} M_p <= k - 1
            vector<int> ind((int)P + 1);
            vector<double> val((int)P + 1);
            int idx = 0;
            for (size_t p = 0; p < P; ++p) {
                ++idx;
                ind[idx] = (int)p + 1;
                val[idx] = (cand[p] ? 1.0 : -1.0);
            }

            glp_add_rows(lp, 1);
            int row = glp_get_num_rows(lp);
            glp_set_mat_row(lp, row, (int)P, ind.data(), val.data());
            glp_set_row_bnds(lp, row, GLP_UP, 0.0, (double)(k - 1));

            ++cuts;
            if (options.verbose) 
                cout << "[ILP] Excluding exact candidate (cut #" << cuts << ")\n";

            if (cuts > maxCuts) {
                if (options.verbose) 
                    cout << "[ILP] Max cuts (" << maxCuts << ") reached.\n";
                break;
            }
        }

        if (found) {
            result.isReachable = true;
            result.optMarking = bestM;
            result.optObj = bestObj;
        } else {
            result.isReachable = false;
        }

    } catch (const exception& e) {
        cerr << "[ILP][ERROR] Exception: " << e.what() << endl;
        result.isReachable = false;
    }

    if (lp) {
        glp_delete_prob(lp);
        lp = nullptr;
    }
    
    result.timeSec = chrono::duration<double>(chrono::high_resolution_clock::now() - t_start).count();
    return result;
}

// -------------------- Public entrypoint --------------------
IlpResult solveILP(const Model& model,
                  const BddResult& bddResult,
                  const IlpOptions& options) {
    if (options.mode == IlpMode::DEADLOCK) 
        return solveDeadlockILP(model, bddResult, options);
    if (options.mode == IlpMode::OPTIMIZATION) 
        return solveOptimizationILP(model, bddResult, options);
    return IlpResult();
}

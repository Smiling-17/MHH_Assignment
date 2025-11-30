#include "reachability.h"
#include <queue>        // cho BFS
#include <stack>        // cho DFS
#include <unordered_set>
#include <iostream>

using namespace std;

ExplicitReachability::ExplicitReachability(const Model &model) : petri_net(model) {}


//BFS
ReachResult ExplicitReachability::computeBFS() {
    ReachResult result;

    double t0 = getTimeSec();
    double m0 = getMemoryMB();

    queue<Marking> q;
    unordered_set<Marking, MarkingHash> visited;

    Marking start = petri_net.M0; // bắt đầu từ marking đầu
    q.push(start);
    visited.insert(start);

    //cout << "[BFS] Bat dau tu: " << toString(start) << endl;

    while (!q.empty()) {
        Marking current = q.front(); // lấy marking đầu hàng đợi
        q.pop(); // xóa marking khỏi hàng đợi

        for (int i = 0; i < (int)petri_net.transitions.size(); i++) { // chạy tất cả transition
            if (isEnabled(petri_net, current, i)) { // kiểm tra transition chạy đc hay ko
                Marking next = fire(petri_net, current, i) ; // chạy transition để đc marking mới

                if (visited.find(next) == visited.end()) {
                    visited.insert(next); // đánh dấu là đã ghé qua
                    q.push(next); // xong sau đó đưa vào hàng đợi
                }
            }
        }
    }


    result.timeSec = getTimeSec() - t0;
    double memNow = getMemoryMB();
    double deltaMem = memNow - m0;
    if(deltaMem < 0) {
        deltaMem = memNow;
    }
    result.memMB = deltaMem;
    result.states = visited.size();


    cout << "[BFS] result: " << result.states
         << " trang thai, " << result.timeSec << " seconds, "
         << result.memMB << " MB" << endl; 

    return result;
}

//DFS
ReachResult ExplicitReachability::computeDFS() {
    ReachResult result;
    double t0 = getTimeSec();
    double m0 = getMemoryMB();

    stack<Marking> s;
    unordered_set<Marking, MarkingHash> visited;

    Marking start = petri_net.M0;
    s.push(start);
    visited.insert(start);

    cout << "[DFS] Bat dau tu: " << toString(start) << endl;

    while (!s.empty()) {
        Marking current = s.top(); // lấy marking trên cùng
        s.pop();    // xóa marking khỏi ngăn xếp

        for(int i = 0; i < (int)petri_net.transitions.size(); i++){
            if (isEnabled(petri_net, current, i)) {
                Marking next = fire(petri_net, current, i);

                if (visited.find(next) == visited.end()) {
                    visited.insert(next);   // đánh dấu đã ghé
                    s.push(next); // thêm vào ngăn xếp
                }
            }
        }
    }

    result.timeSec = getTimeSec() - t0;
    double memNow = getMemoryMB();
    double deltaMem = memNow - m0;
    if (deltaMem < 0) {
        deltaMem = memNow;
    }
    result.memMB = deltaMem;
    result.states = visited.size();

    cout << "[DFS] result: " << result.states
         << " trang thai, " << result.timeSec << " seconds, "
         << result.memMB << " MB" << endl; 

    return result;
}

ReachResult explicitReach(const Model &model, const ReachOptions &opts) {
    ExplicitReachability analyzer(model);
    if (opts.useBFS) {
        return analyzer.computeBFS();
    } else {
        return analyzer.computeDFS();
    }
}

// int main() {
//     // ... (phần parse file PNML để lấy Model)
    
//     // Tạo đối tượng explicit reachability
//     ExplicitReachability analyzer(model);
    
//     // Chạy BFS
//     cout << "=== CHAY BFS ===" << endl;
//     ReachResult bfs_result = analyzer.computeBFS();
    
//     // Chạy DFS
//     cout << "=== CHAY DFS ===" << endl;
//     ReachResult dfs_result = analyzer.computeDFS();
    
//     // So sánh kết quả
//     cout << "=== KET QUA ===" << endl;
//     cout << "BFS: " << bfs_result.states << " trang thai" << endl;
//     cout << "DFS: " << dfs_result.states << " trang thai" << endl;
    
//     return 0;
// }
#include <iostream>
#include <cassert>
#include "reachability.h" // Code của Hậu
#include "utils.h"

Model createDiamondModel() {
    Model m;
    // 4 Places, 2 Transitions
    m.places = {"p0", "p1", "p2", "p3"};
    m.transitions = {"t0", "t1"};
    m.placeIndex = {{"p0",0}, {"p1",1}, {"p2",2}, {"p3",3}};
    m.transIndex = {{"t0",0}, {"t1",1}};

    // Init matrix (4 places x 2 transitions)
    m.Pre.assign(4, vector<int>(2, 0));
    m.Post.assign(4, vector<int>(2, 0));

    // t0: p0 -> {p1, p2}
    m.Pre[0][0] = 1;
    m.Post[1][0] = 1; m.Post[2][0] = 1;

    // t1: {p1, p2} -> p3
    m.Pre[1][1] = 1; m.Pre[2][1] = 1;
    m.Post[3][1] = 1;

    // M0: {p0}
    m.M0 = {1, 0, 0, 0};
    return m;
}

int main() {
    Model m = createDiamondModel();
    ReachOptions opts;
    // opts.dump = true; // Bật nếu muốn in ra màn hình

    cout << "Testing Explicit Reachability..." << endl;
    ReachResult res = explicitReach(m, opts);

    // Kiểm tra kết quả
    cout << "States found: " << res.states << endl;
    
    // Mạng Diamond có 3 trạng thái: {p0}, {p1,p2}, {p3}
    assert(res.states == 3); 

    // Kiểm tra performance (không được âm)
    assert(res.timeSec >= 0.0);
    assert(res.memMB >= 0.0);

    cout << "✅ [PASS] Explicit BFS/DFS đếm đúng số trạng thái!" << endl;
    return 0;
}
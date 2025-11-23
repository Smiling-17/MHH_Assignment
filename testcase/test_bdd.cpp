#include <iostream>
#include <cassert>
#include "bdd.h" // Code của Khoa
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
    BddOptions opts;

    cout << "Testing BDD Reachability..." << endl;
    BddResult res = bddReach(m, opts);

    cout << "States found (BDD): " << res.states << endl;
    
    // Mạng Diamond phải có 3 trạng thái
    assert(res.states == 3);
    
    // Quan trọng: Phải trả về con trỏ BDD để ILP dùng
    if (res.internalState == nullptr) {
        cerr << "❌ [FAIL] BDD Result chưa có internalState (null)!" << endl;
        return 1;
    }

    cout << "✅ [PASS] BDD Symbolic hoạt động đúng!" << endl;
    return 0;
}
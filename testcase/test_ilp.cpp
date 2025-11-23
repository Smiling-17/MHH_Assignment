#include <iostream>
#include <cassert>
#include "ilp.h" // Code của Khôi
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
    BddResult mockBdd; // Dummy input

    cout << "Testing ILP Solver..." << endl;

    // --- TEST 1: Deadlock Detection ---
    IlpOptions opts1;
    opts1.mode = IlpMode::DEADLOCK; // Giả sử enum này có trong ilp.h
    
    IlpResult res1 = solveILP(m, mockBdd, opts1);

    // Mạng Diamond CHẮC CHẮN có deadlock tại p3 (index 3)
    Marking expectedDeadlock = {0, 0, 0, 1};
    
    if (res1.hasDeadlock && res1.deadlockMarking == expectedDeadlock) {
         cout << "   -> Deadlock check: OK" << endl;
    } else {
         cerr << "❌ [FAIL] Deadlock sai. Kì vọng: [0,0,0,1]" << endl;
         return 1;
    }

    // --- TEST 2: Optimization ---
    // Mục tiêu: Maximize (1*p0 + 1*p1 + 1*p2 + 10*p3)
    // Rõ ràng trạng thái {p3} sẽ có giá trị cao nhất là 10.
    IlpOptions opts2;
    opts2.mode = IlpMode::OPTIMIZATION;
    opts2.weights = {1, 1, 1, 10}; // Trọng số cho p0..p3

    IlpResult res2 = solveILP(m, mockBdd, opts2);

    if (res2.optObj == 10.0 && res2.optMarking == expectedDeadlock) {
         cout << "   -> Optimization check: OK" << endl;
    } else {
         cerr << "❌ [FAIL] Opt sai. Kì vọng 10.0, Nhận được: " << res2.optObj << endl;
         return 1;
    }

    cout << "✅ [PASS] ILP Solver hoạt động chuẩn xác!" << endl;
    return 0;
}
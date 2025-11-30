// Đổi tên BDD_H thành MY_ASSIGNMENT_BDD_H để tránh trùng với thư viện
#ifndef MY_ASSIGNMENT_BDD_H  
#define MY_ASSIGNMENT_BDD_H

#include "utils.h"

struct BddOptions {
    int maxIters = 1000;
    bool useGC = true;
};

// Khai báo hàm chính - Task 3: Symbolic Reachability
BddResult bddReach(const Model& net, const BddOptions& opts);

// Hàm kiểm tra marking có reachable không (dùng cho Task 4 & 5)
// Trả về true nếu marking M thuộc tập reachable states trong BDD
bool bdd_check_reachable(const BddResult& bddResult, const Marking& M, int numPlaces);

// Hàm dọn dẹp BDD result (giải phóng memory)
void bdd_cleanup(BddResult& bddResult);

#endif // MY_ASSIGNMENT_BDD_H
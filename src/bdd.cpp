#include "bdd.h"
#include "utils.h"
#include "buddy/bdd.h" // Thư viện BuDDy
#include <iostream>
#include <chrono> // Thư viện để đo thời gian chạy (Task Outcome)

// Hàm helper: Chuyển vector Marking sang BDD node
// (Bỏ static để có thể dùng trong bdd_check_reachable)
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

BddResult bddReach(const Model& net, const BddOptions& opts) {
    // 1. BẮT ĐẦU ĐO THỜI GIAN
    auto start_time = std::chrono::high_resolution_clock::now();

    BddResult res;
    int numPlaces = net.places.size();
    int numVars = numPlaces * 2;

    // 2. KHỞI TẠO BUDDY 
    // Cấp phát bộ nhớ: 1 triệu node, cache 100k
    if (!bdd_isrunning()) {
        bdd_init(1000000, 100000);
        bdd_setvarnum(numVars);
    }

    // 3. ENCODE TRẠNG THÁI ĐẦU (M0)
    bdd M0_bdd = markingToBdd(net.M0, numPlaces);

    // 4. ENCODE TRANSITION RELATION (TR)
    // TR = OR ( Trans_t ) với mọi t
    bdd TR = bdd_false();

    // Tạo cặp biến để đổi tên (x' -> x) sau mỗi bước duyệt
    bddPair* pairs = bdd_newpair();
    for (int i = 0; i < numPlaces; ++i) {
        // Map biến lẻ (Next - 2i+1) về biến chẵn (Curr - 2i)
        bdd_setpair(pairs, i * 2 + 1, i * 2);
    }

    for (size_t t = 0; t < net.transitions.size(); ++t) {
        bdd trans_t = bdd_true();
        for (int p = 0; p < numPlaces; ++p) {
            int curr = p * 2;     // Biến hiện tại x
            int next = p * 2 + 1; // Biến tiếp theo x'

            int preVal = net.Pre[p][t];
            int postVal = net.Post[p][t];

            // --- Logic chuyển đổi trạng thái ---

            // Điều kiện Enable: Pre[p] = 1 -> x phải là 1
            if (preVal == 1) {
                trans_t &= bdd_ithvar(curr);
            }

            // Trạng thái sau khi bắn (Next state logic):
            if (postVal == 1) {
                // Được sinh token -> x' = 1
                trans_t &= bdd_ithvar(next);
            }
            else if (preVal == 1 && postVal == 0) {
                // Bị mất token -> x' = 0
                trans_t &= bdd_nithvar(next);
            }
            else {
                // Place này không liên quan -> Giữ nguyên trạng thái
                // x <-> x' (Bi-implication)
                trans_t &= bdd_apply(bdd_ithvar(curr), bdd_ithvar(next), bddop_biimp);
            }
        }
        // Gộp transition này vào tổng thể
        TR |= trans_t;
    }

    // 5. VÒNG LẶP FIXPOINT (Symbolic Reachability)
    bdd Reached = M0_bdd;
    bdd New = M0_bdd;

    // Tạo tập biến "Hiện tại" (Current Vars - biến chẵn) 
    // Dùng cho phép toán Relational Product và để đếm số trạng thái chính xác
    bdd currentVarSet = bdd_true();
    for (int i = 0; i < numPlaces; ++i) {
        currentVarSet &= bdd_ithvar(i * 2);
    }

    int loopCount = 0;
    while (true) {
        loopCount++;
        if (loopCount > opts.maxIters) break;

        // Tính trạng thái tiếp theo (Image Computation)
        // Next(x') = Exist_x ( New(x) AND TR(x, x') )
        // Relational Product: AND rồi Existential Quantification biến x
        bdd next_prime = bdd_relprod(New, TR, currentVarSet);

        // Đổi tên biến (x' -> x) để chuẩn bị cho vòng lặp sau
        bdd next_state = bdd_replace(next_prime, pairs);

        // Kiểm tra xem có trạng thái mới không?
        bdd new_diff = next_state - Reached;

        // Nếu tập mới là rỗng (False) -> Đã hội tụ, dừng lặp
        if (new_diff == bdd_false()) {
            break;
        }

        // Cập nhật tập Reached
        Reached |= new_diff;     // Gộp cái mới vào cái đã biết
        New = new_diff;          // Chỉ dùng cái mới để duyệt tiếp vòng sau
    }

    // 6. TỔNG HỢP KẾT QUẢ

    // Đếm tổ hợp của biến hiện tại
    res.states = (size_t)bdd_satcountset(Reached, currentVarSet);
    res.nodeCount = bdd_getnodenum();
    res.iters = loopCount;

    // KẾT THÚC ĐO THỜI GIAN
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    res.timeSec = elapsed.count(); // Lưu thời gian thực (giây)

    // Lưu internalState
    // Lấy ID số nguyên của node BDD gốc
    int reachedID = Reached.id();

    // Tăng bộ đếm tham chiếu để BuDDy không tự động xóa node này (Reference Counting)
    bdd_addref(reachedID); 

    // Cấp phát vùng nhớ int để lưu ID này và chuyển thành void*
    int* storedBDD = new int(reachedID);
    res.internalState = (void*)storedBDD;

    // Dọn dẹp memory phụ trợ 
    bdd_freepair(pairs);  

    return res;
}

// ============================================================
// Hàm kiểm tra marking có reachable không (Task 4 & 5)
// ============================================================
bool bdd_check_reachable(const BddResult& bddResult, const Marking& M, int numPlaces) {
    // Kiểm tra BDD đã được khởi tạo chưa
    if (bddResult.internalState == nullptr) {
        std::cerr << "[BDD] Warning: internalState is null, cannot check reachability" << std::endl;
        return false;
    }
    
    // Kiểm tra BuDDy đang chạy
    if (!bdd_isrunning()) {
        std::cerr << "[BDD] Warning: BuDDy not running" << std::endl;
        return false;
    }
    
    // Lấy lại BDD ID từ internalState
    int* storedID = static_cast<int*>(bddResult.internalState);
    bdd Reached = bdd_addref(*storedID);
    
    // Convert marking sang BDD node
    bdd markingBdd = markingToBdd(M, numPlaces);
    
    // Kiểm tra: marking AND Reached != FALSE
    // Nếu AND ra FALSE nghĩa là marking không thuộc tập Reached
    bdd result = markingBdd & Reached;
    
    bool isReachable = (result != bdd_false());
    
    // Giải phóng reference đã thêm
    bdd_delref(*storedID);
    
    return isReachable;
}

// ============================================================
// Hàm dọn dẹp BDD result
// ============================================================
void bdd_cleanup(BddResult& bddResult) {
    if (bddResult.internalState != nullptr) {
        int* storedID = static_cast<int*>(bddResult.internalState);
        
        // Giảm reference count trong BuDDy
        if (bdd_isrunning()) {
            bdd_delref(*storedID);
        }
        
        // Giải phóng memory
        delete storedID;
        bddResult.internalState = nullptr;
    }
}
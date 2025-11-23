#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstdint>

// [FIX] Thêm thư viện này để dùng sysconf trên Linux/Mac
#ifdef __linux__
#include <unistd.h> 
#endif

using namespace std;

// ============================================================
//  File: utils.h
//  Mô tả: Định nghĩa cấu trúc dữ liệu và hàm tiện ích dùng chung
// ============================================================

// ======== 1️⃣ Kiểu dữ liệu cơ bản ========
using Token = uint8_t;                  
using Marking = vector<Token>;          

// ======== 2️⃣ Cấu trúc Model ========
struct Model {
    vector<string> places;
    vector<string> transitions;
    vector<vector<int>> Pre;
    vector<vector<int>> Post;
    Marking M0;
    unordered_map<string, int> placeIndex;
    unordered_map<string, int> transIndex;
};

// ======== 3️⃣ Kết quả trung gian của các module ========

struct ReachResult {
    size_t states = 0;
    double timeSec = 0.0;
    double memMB = 0.0;
};

struct BddResult {
    size_t states = 0;
    double timeSec = 0.0;
    double memMB = 0.0;
    int nodeCount = 0;
    int iters = 0;

    // [FIX] Thêm con trỏ này để truyền dữ liệu sang ILP mà không lộ type thư viện
    // Khoa sẽ gán pointer của bdd_manager hoặc ddd node vào đây
    void* internalState = nullptr; 
};

struct IlpResult {
    bool hasDeadlock = false;
    bool isReachable = false; // [FIX] Bổ sung flag: Deadlock này có reachable không?
    Marking deadlockMarking;
    Marking optMarking;
    double optObj = 0.0;
    double timeSec = 0.0;
};

// ======== 4️⃣ Hàm tiện ích cho Marking ========

inline bool operator==(const Marking& a, const Marking& b) {
    return a.size() == b.size() && equal(a.begin(), a.end(), b.begin());
}

struct MarkingHash {
    size_t operator()(const Marking& m) const {
        size_t h = 0;
        for (auto v : m)
            h = h * 131 + v; 
        return h;
    }
};

inline string toString(const Marking& M) {
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < M.size(); ++i) {
        oss << (int)M[i]; // Ép kiểu int để in đúng số
        if (i + 1 < M.size()) oss << ",";
    }
    oss << "]";
    return oss.str();
}

// ======== 5️⃣ Hàm logic của Petri Net ========

inline bool isEnabled(const Model& net, const Marking& M, int t) {
    for (size_t p = 0; p < net.places.size(); ++p) {
        if (M[p] < net.Pre[p][t]) return false; 
    }
    return true;
}

inline Marking fire(const Model& net, const Marking& M, int t) {
    Marking M2 = M; // Copy ra marking mới
    for (size_t p = 0; p < net.places.size(); ++p) {
        M2[p] = M[p] - net.Pre[p][t] + net.Post[p][t];
    }
    return M2;
}

// ======== 6️⃣ Hàm thống kê bộ nhớ và thời gian ========

inline double getMemoryMB() {
    #ifdef __linux__
        long rss = 0L;
        FILE* fp = fopen("/proc/self/statm", "r");
        if (fp == nullptr) return 0.0;
        if (fscanf(fp, "%*s%ld", &rss) != 1) { fclose(fp); return 0.0; }
        fclose(fp);
        return (rss * sysconf(_SC_PAGESIZE)) / (1024.0 * 1024.0);
    #else
        return 0.0; // Windows/Mac (phức tạp hơn, tạm bỏ qua hoặc implement sau)
    #endif
}

inline double getTimeSec() {
    using namespace chrono;
    // Dùng static để lần gọi đầu tiên làm mốc
    static auto t_start = high_resolution_clock::now(); 
    auto t_now = high_resolution_clock::now();
    duration<double> diff = t_now - t_start;
    return diff.count();
}

#endif // UTILS_H
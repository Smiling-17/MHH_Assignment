#include <iostream>
#include <fstream>
#include <cassert>
#include "parser.h" // Code của Quỳnh
#include "utils.h"

int main() {
    // 1. Tạo file PNML mẫu (Mạng đơn giản: p1 -> t1 -> p2)
    std::ofstream out("test_simple.pnml");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<pnml><net><page>\n"
        << " <place id=\"p1\"><initialMarking><text>1</text></initialMarking></place>\n"
        << " <place id=\"p2\"></place>\n"
        << " <transition id=\"t1\"></transition>\n"
        << " <arc id=\"a1\" source=\"p1\" target=\"t1\"/>\n"
        << " <arc id=\"a2\" source=\"t1\" target=\"p2\"/>\n"
        << "</page></net></pnml>";
    out.close();

    try {
        cout << "Testing Parser..." << endl;
        Model m = parsePNML("test_simple.pnml");

        // 2. Kiểm tra số lượng
        assert(m.places.size() == 2);
        assert(m.transitions.size() == 1);

        // 3. Kiểm tra Map ID
        assert(m.placeIndex.count("p1") && m.placeIndex.count("p2"));
        int p1_idx = m.placeIndex["p1"];
        int p2_idx = m.placeIndex["p2"];
        int t1_idx = m.transIndex["t1"];

        // 4. Kiểm tra Marking ban đầu (p1=1, p2=0)
        assert(m.M0[p1_idx] == 1);
        assert(m.M0[p2_idx] == 0);

        // 5. Kiểm tra Ma trận Pre/Post
        // t1 lấy từ p1 (Pre[p1][t1] == 1)
        assert(m.Pre[p1_idx][t1_idx] == 1);
        // t1 đẩy vào p2 (Post[p2][t1] == 1)
        assert(m.Post[p2_idx][t1_idx] == 1);

        cout << "✅ [PASS] Parser hoạt động chính xác!" << endl;
        remove("test_simple.pnml"); // Xóa file tạm
    } catch (const exception& e) {
        cerr << "❌ [FAIL] Parser lỗi: " << e.what() << endl;
        return 1;
    }
    return 0;
}
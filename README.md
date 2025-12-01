# 🔄 Petri Net Solver

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10%2B-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> **Symbolic and Algebraic Reasoning in Petri Nets**  
> Bài tập lớn môn Mô hình hóa toán học (CO2011) - HCMUT

Hệ thống phân tích Petri Net hoàn chỉnh cho 1-safe nets, hỗ trợ explicit reachability (BFS/DFS), symbolic reachability (BDD), deadlock detection và optimization.

---

## 📋 Mục lục

- [Tính năng](#-tính-năng)
- [Cấu trúc Project](#-cấu-trúc-project)
- [Yêu cầu hệ thống](#-yêu-cầu-hệ-thống)
- [Hướng dẫn cài đặt](#-hướng-dẫn-cài-đặt)
- [Hướng dẫn sử dụng](#-hướng-dẫn-sử-dụng)
- [Input/Output Format](#-inputoutput-format)
- [Các Task đã triển khai](#-các-task-đã-triển-khai)
- [Thư viện sử dụng](#-thư-viện-sử-dụng)
- [Test Cases](#-test-cases)
- [Nhóm thực hiện](#-nhóm-thực-hiện)

---

## ✨ Tính năng

| Task | Mô tả | Trạng thái |
|------|-------|------------|
| **Task 1** | PNML Parser - Đọc file Petri Net Markup Language | ✅ |
| **Task 2** | Explicit Reachability - BFS và DFS | ✅ |
| **Task 3** | Symbolic Reachability - BDD (Binary Decision Diagrams) | ✅ |
| **Task 4** | Deadlock Detection - ILP + BDD | ✅ |
| **Task 5** | Optimization - Maximize objective function | ✅ |

---

## 📁 Cấu trúc Project

```
Assignment_Group90/
├── 📂 src/                      # Source code
│   ├── main.cpp                 # CLI và điều phối
│   ├── parser.cpp/h             # Task 1: PNML Parser
│   ├── reachability.cpp/h       # Task 2: BFS/DFS
│   ├── bdd.cpp/h                # Task 3: Symbolic BDD
│   ├── ilp.cpp/h                # Task 4 & 5: ILP với GLPK
│   ├── utils.h                  # Cấu trúc dữ liệu dùng chung
│   ├── tinyxml2.cpp/h           # XML Parser library
│   ├── buddy/                   # BuDDy BDD library
│   └── CMakeLists.txt           # Build configuration
│
├── 📂 data/                     # Test PNML files
│   ├── 01_minimal.pnml
│   ├── 02_deadlock_simple.pnml
│   ├── ...
│   └── 15_large_state_space.pnml
│
├── 📂 bin/                      # Executables
│   └── petri_solver.exe
│
├── 📂 output/                   # Kết quả chạy
│   ├── result.csv               # Bảng hiệu năng
│   ├── deadlock.txt             # Deadlock marking
│   ├── optimum.txt              # Optimal marking
│   └── petri_net.dot            # GraphViz visualization
│
├── 📂 scripts/                  # Automation scripts
│   ├── run_all_tests.ps1        # Chạy tất cả test cases
│   └── analyze_results.py       # Tạo biểu đồ
│
├── 📂 testcase/                 # Unit tests
├── 📂 latex/                    # LaTeX report source
├── 📂 report/                   # PDF report
└── README.md                    # File này
```

---

## 💻 Yêu cầu hệ thống

### Bắt buộc
- **C++ Compiler**: GCC 7+ / MSVC 2019+ / Clang 8+ (hỗ trợ C++17)
- **CMake**: 3.10 trở lên

### Tùy chọn (cho Task 4 & 5)
- **GLPK**: GNU Linear Programming Kit 5.0+
  - Windows: Download từ [GLPK for Windows](https://sourceforge.net/projects/winglpk/)
  - Linux: `sudo apt install libglpk-dev`
  - macOS: `brew install glpk`

### Tùy chọn (cho visualization)
- **Python 3.8+** với matplotlib, pandas (xem `scripts/requirements.txt`)
- **GraphViz** để render file DOT

---

## 🔧 Hướng dẫn cài đặt

### 1. Clone repository

```bash
git clone <repository-url>
cd Assignment_Group90
```

### 2. Build với CMake

**Windows (PowerShell):**
```powershell
cd src
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

**Linux/macOS:**
```bash
cd src
mkdir build && cd build
cmake ..
make -j4
```

### 3. Kiểm tra build

```bash
# Executable sẽ được tạo tại bin/petri_solver.exe (Windows)
# hoặc bin/petri_solver (Linux/macOS)
```

> **Lưu ý**: Nếu GLPK không được tìm thấy, CMake sẽ hiển thị warning và Task 4 & 5 sẽ bị disable. Project vẫn build thành công.

---

## 🚀 Hướng dẫn sử dụng

### Cú pháp cơ bản

```bash
./bin/petri_solver --input <file.pnml> [options]
```

### Các tùy chọn

| Option | Mô tả | Mặc định |
|--------|-------|----------|
| `--input <file>` | Đường dẫn file PNML **(Bắt buộc)** | - |
| `--mode <mode>` | `explicit`, `bdd`, hoặc `all` | `all` |
| `--optimize` | Bật Task 5 (Optimization) | Tắt |
| `--outdir <path>` | Thư mục lưu kết quả | `output/` |
| `--help` | Hiển thị hướng dẫn | - |

### Ví dụ sử dụng

```bash
# Chạy tất cả tasks trên một file
./bin/petri_solver --input data/simple_test.pnml --mode all

# Chỉ chạy BDD symbolic reachability
./bin/petri_solver --input data/05_parallel.pnml --mode bdd

# Chạy với optimization
./bin/petri_solver --input data/simple_test.pnml --mode bdd --optimize

# Chạy tất cả test cases (Windows PowerShell)
.\scripts\run_all_tests.ps1

# Tạo biểu đồ từ kết quả
python scripts\analyze_results.py
```

---

## 📄 Input/Output Format

### Input: PNML File

File PNML theo chuẩn [Petri Net Markup Language](http://www.pnml.org/):

```xml
<?xml version="1.0" encoding="UTF-8"?>
<pnml xmlns="http://www.pnml.org/version-2009/grammar/pnml">
  <net id="example" type="http://www.pnml.org/version-2009/grammar/ptnet">
    <page id="page1">
      <place id="p0">
        <initialMarking><text>1</text></initialMarking>
      </place>
      <place id="p1"/>
      <transition id="t0"/>
      <arc id="a1" source="p0" target="t0"/>
      <arc id="a2" source="t0" target="p1"/>
    </page>
  </net>
</pnml>
```

### Output Files

| File | Mô tả |
|------|-------|
| `result.csv` | Bảng tổng hợp: Model, States, Time, BDD Nodes, Deadlock, Optimization |
| `deadlock.txt` | Chi tiết deadlock marking (nếu tìm thấy) |
| `optimum.txt` | Chi tiết optimal marking và objective value |
| `petri_net.dot` | Đồ thị Petri Net (mở với GraphViz hoặc online viewer) |

### Ví dụ output console

```
[PARSER] Opening file: data/simple_test.pnml
[PARSER] Found 4 places, 3 transitions, 7 arcs
[BFS] result: 4 trang thai, 0.00012 seconds
[DFS] result: 4 trang thai, 0.00010 seconds
[BDD] Fixpoint converged in 3 iterations
[BDD] Reachable states: 4, BDD nodes: 25
[ILP] Deadlock found: [0,0,0,1]
[ILP] Optimal marking: [0,1,1,0] with objective = 2
```

---

## 📚 Các Task đã triển khai

### Task 1: PNML Parser
- Sử dụng **TinyXML2** để parse XML
- Hỗ trợ `<place>`, `<transition>`, `<arc>`, `<initialMarking>`
- Xây dựng ma trận `Pre[p][t]` và `Post[p][t]`
- Export đồ thị DOT cho visualization

### Task 2: Explicit Reachability
- **BFS**: Sử dụng `std::queue`, duyệt theo chiều rộng
- **DFS**: Sử dụng `std::stack`, duyệt theo chiều sâu
- Hash function: Polynomial rolling hash (`h = h * 131 + v`)
- Lưu visited markings trong `std::unordered_set`

### Task 3: Symbolic Reachability (BDD)
- Sử dụng thư viện **BuDDy 2.4**
- Encoding: 2n biến (current + next state)
- Transition relation: OR của tất cả transitions
- Fixpoint computation với `bdd_relprod()` và `bdd_replace()`

### Task 4: Deadlock Detection
- Mô hình **ILP** với **GLPK**
- Biến: `M[p] ∈ {0,1}` cho mỗi place
- Ràng buộc: Không đủ token để fire bất kỳ transition nào
- Kết hợp **BDD** để verify reachability (cutting-plane)

### Task 5: Optimization
- Maximize `c^T · M` trên reachable markings
- Cutting-plane method: Loại trừ candidates không reachable
- Tích hợp với BDD qua `bdd_check_reachable()`

---

## 📦 Thư viện sử dụng

| Thư viện | Phiên bản | Mục đích | Nguồn |
|----------|-----------|----------|-------|
| **TinyXML2** | - | Parse XML/PNML | [GitHub](https://github.com/leethomason/tinyxml2) |
| **BuDDy** | 2.4 | BDD operations | [SourceForge](http://buddy.sourceforge.net/) |
| **GLPK** | 5.0+ | ILP Solver | [GNU](https://www.gnu.org/software/glpk/) |

> **Lưu ý**: TinyXML2 và BuDDy được tích hợp sẵn trong `src/`. Chỉ cần cài GLPK nếu muốn sử dụng Task 4 & 5.

---

## 🧪 Test Cases

Bộ test gồm 17 file PNML trong folder `data/`:

| # | File | Mô tả |
|---|------|-------|
| 1 | `01_minimal.pnml` | Mô hình tối giản (2 places, 1 transition) |
| 2 | `02_deadlock_simple.pnml` | Có deadlock đảm bảo |
| 3 | `03_loop_simple.pnml` | Vòng lặp vô hạn |
| 4 | `04_choice.pnml` | Non-deterministic choice |
| 5 | `05_parallel.pnml` | Thực thi song song |
| 6 | `06_mutex.pnml` | Mutual exclusion |
| 7 | `07_dining_philosophers_2.pnml` | Dining Philosophers (2 người) |
| 8 | `08_producer_consumer.pnml` | Producer-Consumer |
| 9 | `09_pipeline_3stage.pnml` | Pipeline 3 giai đoạn |
| 10 | `10_complex_workflow.pnml` | Workflow phức tạp |
| 11 | `11_readers_writers.pnml` | Readers-Writers problem |
| 12 | `12_state_machine.pnml` | State machine |
| 13 | `13_token_ring.pnml` | Token ring protocol |
| 14 | `14_concurrent_tasks.pnml` | Concurrent tasks |
| 15 | `15_large_state_space.pnml` | State space lớn |

### Chạy tất cả tests

```powershell
# Windows
.\scripts\run_all_tests.ps1

# Sau đó tạo biểu đồ
python scripts\analyze_results.py
```

---

## 👥 Nhóm thực hiện

**Group 90** - Trường Đại học Bách Khoa TP.HCM (HCMUT)

| STT | Họ và tên | MSSV |
|-----|-----------|------|
| 1 | [Họ tên thành viên 1] | [MSSV] |
| 2 | [Họ tên thành viên 2] | [MSSV] |
| 3 | [Họ tên thành viên 3] | [MSSV] |
| 4 | [Họ tên thành viên 4] | [MSSV] |
| 5 | [Họ tên thành viên 5] | [MSSV] |

**Môn học**: Mô hình hóa toán học (CO2011)  
**Học kỳ**: HK2 - Năm học 2024-2025

---

## 📝 License

MIT License - Xem file [LICENSE](LICENSE) để biết thêm chi tiết.

---

<p align="center">
  Made with ❤️ by Group 90 @ HCMUT
</p>


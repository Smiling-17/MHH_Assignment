// ============================================================
//  File: src/main.cpp
//  Người thực hiện: Nguyễn Tiến Dũng (Leader)
//  Mô tả: Chương trình điều phối chính (Pipeline) cho Assignment
//  Reference: Assignment_MHH.pdf & Chia việc BTL.docx
// ============================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <cstdlib>

// Include file tiện ích chung
#include "utils.h"

// Include header của các thành viên (Giả định các file này đã có)
#include "parser.h"       
#include "reachability.h" 
#include "bdd.h"          
#include "ilp.h"         

using namespace std;

// Hàm in hướng dẫn sử dụng (Help)
void printUsage() {
    cout << "Usage: ./petri_solver --input <file.pnml> [options]\n";
    cout << "Options:\n";
    cout << "  --input <file>     : Path to input PNML file (Required)\n";
    cout << "  --mode <mode>      : 'explicit', 'bdd', or 'all' (Default: all)\n";
    cout << "  --optimize         : Enable ILP Optimization (Task 5)\n";
    cout << "  --outdir <path>    : Directory to save results (Default: output/)\n";
    cout << "  --help             : Show this help message\n";
    cout << "Example:\n";
    cout << "  ./petri_solver --input data/simple_test.pnml --mode bdd --optimize\n";
}

int main(int argc, char* argv[]) {
    // --------------------------------------------------------
    // 1. XỬ LÝ THAM SỐ DÒNG LỆNH (CLI Parsing)
    // --------------------------------------------------------
    string inputFile;
    string mode = "all";
    string outDir = "output/";
    bool doOptimize = false;

    if (argc < 2) {
        printUsage();
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--input") == 0 && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = argv[++i];
        } else if (strcmp(argv[i], "--outdir") == 0 && i + 1 < argc) {
            outDir = argv[++i];
        } else if (strcmp(argv[i], "--optimize") == 0) {
            doOptimize = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            printUsage();
            return 0;
        }
    }

    if (inputFile.empty()) {
        cerr << "[ERROR] Missing input file! Please use --input.\n";
        return 1;
    }

    // Đảm bảo thư mục output tồn tại
    string cmd = "mkdir -p " + outDir;
    int ret = system(cmd.c_str());
    (void)ret; // Tránh warning unused variable

    // 2. CHUẨN BỊ FILE CSV (result.csv)
    // Format chuẩn: Model,Method,States,TimeSec,MemMB,Deadlock,OptObj,OptMarking
    string csvPath = outDir + "result.csv";
    bool fileExists = false;
    ifstream checkFile(csvPath);
    if (checkFile.good()) fileExists = true;
    checkFile.close();

    ofstream csvFile(csvPath, ios::app); // Mở chế độ append (ghi nối tiếp)
    if (!csvFile.is_open()) {
        cerr << "[ERROR] Cannot open " << csvPath << " for writing.\n";
        return 1;
    }

    // Ghi header nếu file mới tạo 
    if (!fileExists) {
        csvFile << "Model,Method,States,TimeSec,MemMB,Deadlock,OptObj,OptMarking\n";
    }

    // Lấy tên file model ngắn gọn để ghi log
    string modelName = inputFile.substr(inputFile.find_last_of("/\\") + 1);

    try {
        // TASK 1: PARSE PNML 
        cout << "[INFO] Parsing PNML: " << inputFile << "..." << endl;
        
        Model model = parsePNML(inputFile);
        
        cout << "[INFO] Parsed successfully. Places: " << model.places.size() 
             << ", Transitions: " << model.transitions.size() << endl;


        // TASK 2: EXPLICIT REACHABILITY
        if (mode == "explicit" || mode == "all") {
            cout << "[INFO] Task 2: Running Explicit Reachability (BFS/DFS)..." << endl;
            
            ReachOptions reachOpts; // Struct rỗng hoặc thêm tùy chọn nếu cần
            ReachResult res = explicitReach(model, reachOpts);
            
            cout << "       -> States: " << res.states 
                 << ", Time: " << res.timeSec << "s" 
                 << ", Mem: " << res.memMB << "MB" << endl;
            
            // Ghi kết quả vào CSV (Các cột Deadlock/Opt để N/A)
            csvFile << modelName << ",Explicit," << res.states << "," 
                    << res.timeSec << "," << res.memMB << ",N/A,N/A,N/A\n";
        }


        // TASK 3: SYMBOLIC REACHABILITY - BDD 
        BddResult bddRes; // Biến này lưu kết quả BDD để dùng cho Task 4
        
        // Nếu mode là explicit thì bỏ qua, nhưng Task 4 cần BDD nên ta vẫn phải chạy
        // hoặc báo lỗi nếu người dùng chỉ chọn Explicit mà đòi tìm Deadlock.
        // Ở đây ta ưu tiên chạy nếu mode == bdd hoặc all.
        
        if (mode == "bdd" || mode == "all") {
            cout << "[INFO] Task 3: Running Symbolic Reachability (BDD)..." << endl;
            
            BddOptions bddOpts;
            bddRes = bddReach(model, bddOpts);
            
            cout << "       -> States: " << bddRes.states 
                 << ", Nodes: " << bddRes.nodeCount 
                 << ", Time: " << bddRes.timeSec << "s" << endl;

            // Ghi phần đầu vào CSV (Chưa xuống dòng vội để ghi tiếp Deadlock/Opt)
            csvFile << modelName << ",BDD," << bddRes.states << "," 
                    << bddRes.timeSec << "," << bddRes.memMB << ",";
        } else {
            // Nếu người dùng chỉ chạy Explicit, ta không có kết quả BDD để ghi CSV dòng này
            // Nhưng để Task 4 chạy được, cần cảnh báo
            cout << "[WARN] Skipping BDD. Deadlock detection might fail or be skipped.\n";
        }


        // TASK 4: DEADLOCK DETECTION (ILP + BDD) 
        // Chỉ chạy nếu đã có kết quả BDD (bddRes.states > 0 hoặc internalState != nullptr)
        if (mode == "bdd" || mode == "all") {
            cout << "[INFO] Task 4: Detecting Deadlock (ILP + BDD)..." << endl;
            
            IlpOptions ilpOpts;
            ilpOpts.mode = IlpMode::DEADLOCK;
            
            // Gọi hàm giải ILP, truyền vào kết quả BDD để kiểm tra reachability
            IlpResult deadlockRes = solveILP(model, bddRes, ilpOpts);
            
            // Mở file output/deadlock.txt để ghi 
            ofstream dlFile(outDir + "deadlock.txt");
            
            if (deadlockRes.hasDeadlock && deadlockRes.isReachable) {
                cout << "       [FOUND] Deadlock at: " << toString(deadlockRes.deadlockMarking) << endl;
                
                // Ghi vào CSV: Deadlock = Yes
                csvFile << "Yes,";
                
                // Ghi vào file text
                dlFile << "Deadlock found: " << toString(deadlockRes.deadlockMarking) << endl;
            } else {
                cout << "       [NONE] No reachable deadlock found." << endl;
                
                // Ghi vào CSV: Deadlock = No
                csvFile << "No,";
                
                // Ghi vào file text
                dlFile << "None" << endl;
            }
            dlFile.close();


            // TASK 5: OPTIMIZATION 
            if (doOptimize) {
                cout << "[INFO] Task 5: Optimizing Objective (Maximize c^T M)..." << endl;
                
                IlpOptions optOpts;
                optOpts.mode = IlpMode::OPTIMIZATION;
                // Mặc định trọng số là 1 cho tất cả place (c = [1,1,...])
                optOpts.weights.assign(model.places.size(), 1); 
                
                IlpResult optRes = solveILP(model, bddRes, optOpts);
                
                // Mở file output/optimum.txt [cite: 156]
                ofstream optFile(outDir + "optimum.txt");
                
                if (optRes.isReachable) {
                    cout << "       -> Max Value: " << optRes.optObj << endl;
                    
                    // Ghi CSV: OptObj, OptMarking (Xuống dòng kết thúc record)
                    csvFile << optRes.optObj << ",\"" << toString(optRes.optMarking) << "\"\n";
                    
                    // Ghi file text
                    optFile << "Max Objective: " << optRes.optObj << "\n";
                    optFile << "Marking: " << toString(optRes.optMarking) << "\n";
                } else {
                    cout << "       [FAIL] Optimization failed or no reachable state." << endl;
                    csvFile << "N/A,N/A\n";
                    optFile << "None\n";
                }
                optFile.close();
            } else {
                // Nếu không chạy Optimize thì điền N/A vào CSV cho đủ cột
                csvFile << "N/A,N/A\n";
            }
        } 

    } catch (const exception& e) {
        cerr << "\n[FATAL ERROR] Exception occurred: " << e.what() << endl;
        csvFile.close();
        return 1;
    }

    cout << "[INFO] All tasks completed. Results saved to '" << outDir << "'." << endl;
    csvFile.close();
    
    return 0;
}
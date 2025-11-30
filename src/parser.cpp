#include <iostream>
#include <fstream>
#include <set>
#include "parser.h"
#include "tinyxml2.h"

using namespace tinyxml2;
using namespace std;

// ======== HELPER FUNCTIONS ========

string resolvePath(const string& filename) {
    // Kiểm tra file có tồn tại không
    ifstream test(filename);
    if (test.good()) {
        test.close();
        return filename;
    }
    
    // Thử với prefix "../" (nếu đang chạy từ thư mục src/)
    string parentPath = "../" + filename;
    ifstream test2(parentPath);
    if (test2.good()) {
        test2.close();
        return parentPath;
    }
    
    return filename;
}

bool createDirectoryIfNotExists(const string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != string::npos) {
        string dir = path.substr(0, pos);
        // Đơn giản: thử tạo file tạm để kiểm tra
        ofstream test(dir + "/test.tmp");
        if (test.good()) {
            test.close();
            remove((dir + "/test.tmp").c_str());
            return true;
        }
    }
    return false;
}

// ======== MAIN PARSER FUNCTION ========

Model parsePNML(const string& filename, bool exportDot, const string& dotPath)
{
    XMLDocument doc;

    // Resolve file path
    string resolvedFilename = resolvePath(filename);
    cout << "[PARSER] Opening file: " << resolvedFilename << endl;

    if (doc.LoadFile(resolvedFilename.c_str()) != XML_SUCCESS) {
        throw runtime_error("Cannot open PNML file: " + resolvedFilename);
    }

    // ==========================
    // 1. Đọc root PNML
    // ==========================

    XMLElement* pnml = doc.FirstChildElement("pnml");
    if (!pnml) throw runtime_error("<pnml> missing!");

    XMLElement* net = pnml->FirstChildElement("net");
    if (!net) throw runtime_error("<net> missing!");

    cout << "[PARSER] Found <pnml> and <net> elements" << endl;

    // Xử lý <page> element nếu có
    XMLElement* page = net->FirstChildElement("page");
    if (page) {
        cout << "[PARSER] Found <page> element, using it for parsing" << endl;
        net = page;
    } else {
        cout << "[PARSER] No <page> element, using <net> directly" << endl;
    }

    // ==========================
    // 2. Thu thập place, trans
    // ==========================

    vector<string> placeIds;
    vector<string> transIds;
    unordered_map<string,int> initialMarks;

    // ---- place ----
    for (XMLElement* p = net->FirstChildElement("place"); p; p = p->NextSiblingElement("place"))
    {
        const char* id = p->Attribute("id");
        if (!id) throw runtime_error("place missing id!");

        string sid = id;

        cout << "[PARSER] Found place: " << sid << endl;

        // check duplicate
        if (find(placeIds.begin(), placeIds.end(), sid) != placeIds.end())
            throw runtime_error("Duplicate place id: " + sid);

        placeIds.push_back(sid);

        // check initial marking
        XMLElement* im = p->FirstChildElement("initialMarking");
        if (im) {
            XMLElement* val = im->FirstChildElement("text");
            if (val && val->GetText()) {
                int m = stoi(val->GetText());
                initialMarks[sid] = m;
                cout << "[PARSER]   - Initial marking: " << m << endl;
            }
        }
    }

    // ---- transition ----
    for (XMLElement* t = net->FirstChildElement("transition"); t; t = t->NextSiblingElement("transition"))
    {
        const char* id = t->Attribute("id");
        if (!id) throw runtime_error("transition missing id!");

        string sid = id;

        cout << "[PARSER] Found transition: " << sid << endl;

        if (find(transIds.begin(), transIds.end(), sid) != transIds.end())
            throw runtime_error("Duplicate transition id: " + sid);

        transIds.push_back(sid);
    }

    // sort để đảm bảo thứ tự consistent
    sort(placeIds.begin(), placeIds.end());
    sort(transIds.begin(), transIds.end());

    // ==========================
    // 3. Tạo Model
    // ==========================

    Model model;
    model.places = placeIds;
    model.transitions = transIds;

    size_t P = placeIds.size();
    size_t T = transIds.size();

    // Khởi tạo ma trận Pre và Post
    model.Pre.assign(P, vector<int>(T, 0));
    model.Post.assign(P, vector<int>(T, 0));
    model.M0.assign(P, 0);

    // Tạo mapping ID → index
    for (size_t i = 0; i < P; i++) model.placeIndex[placeIds[i]] = i;
    for (size_t j = 0; j < T; j++) model.transIndex[transIds[j]] = j;

    // ==========================
    // 4. Xử lý initial marking
    // ==========================

    for (auto& kv : initialMarks)
    {
        if (!model.placeIndex.count(kv.first))
            throw runtime_error("initialMarking refers to unknown place: " + kv.first);
        model.M0[model.placeIndex[kv.first]] = kv.second;
    }

    // ==========================
    // 5. Đọc <arc>
    // ==========================

    size_t arcCount = 0;

    for (XMLElement* a = net->FirstChildElement("arc"); a; a = a->NextSiblingElement("arc"))
    {
        arcCount++;

        const char* sid = a->Attribute("source");
        const char* tid = a->Attribute("target");

        if (!sid || !tid)
            throw runtime_error("arc missing source or target!");

        string s = sid, t = tid;

        cout << "[PARSER] Found arc: " << s << " -> " << t << endl;

        int w = 1; // default weight
        XMLElement* ins = a->FirstChildElement("inscription");
        if (ins) {
            XMLElement* val = ins->FirstChildElement("text");
            if (val && val->GetText()) {
                w = stoi(val->GetText());
                cout << "[PARSER]   - Weight: " << w << endl;
            }
        }

        bool sIsPlace = model.placeIndex.count(s);
        bool tIsPlace = model.placeIndex.count(t);
        bool sIsTrans = model.transIndex.count(s);
        bool tIsTrans = model.transIndex.count(t);

        if (sIsPlace && tIsTrans) {
            // Pre arc: place → transition
            int p = model.placeIndex[s];
            int tt = model.transIndex[t];
            model.Pre[p][tt] = w;
            cout << "[PARSER]   - Added Pre[" << p << "][" << tt << "] = " << w << endl;
        }
        else if (sIsTrans && tIsPlace) {
            // Post arc: transition → place
            int p = model.placeIndex[t];
            int tt = model.transIndex[s];
            model.Post[p][tt] = w;
            cout << "[PARSER]   - Added Post[" << p << "][" << tt << "] = " << w << endl;
        }
        else {
            throw runtime_error("Invalid arc: must be place→transition or transition→place");
        }
    }

    model.arcCount = arcCount;

    // ==========================
    // 6. Export DOT (optional)
    // ==========================

    if (exportDot) {
        cout << "[PARSER] Exporting DOT to: " << dotPath << endl;
        
        // Tạo thư mục nếu chưa tồn tại
        createDirectoryIfNotExists(dotPath);
        
        ofstream out(dotPath);
        if (!out) {
            throw runtime_error("Cannot create DOT file: " + dotPath);
        }

        out << "digraph PN {\n";
        out << " rankdir=LR;\n";
        out << " node [fontname=\"Arial\"];\n\n";

        // Places (với initial marking)
        for (size_t i = 0; i < P; i++) {
            out << "  \"" << model.places[i] << "\" [shape=circle";
            if (model.M0[i] > 0) {
                out << " style=filled fillcolor=lightgray";
            }
            out << " label=\"" << model.places[i];
            if (model.M0[i] > 0) {
                out << "\\n(" << (int)model.M0[i] << ")";
            }
            out << "\"];\n";
        }

        // Transitions
        for (size_t j = 0; j < T; j++)
            out << "  \"" << model.transitions[j] << "\" [shape=box label=\"" << model.transitions[j] << "\"];\n";

        out << "\n";

        // Pre arcs (place → transition)
        for (size_t p = 0; p < P; p++)
            for (size_t t = 0; t < T; t++)
                if (model.Pre[p][t] > 0)
                    out << "  \"" << model.places[p] << "\" -> \"" << model.transitions[t]
                        << "\" [label=\"" << model.Pre[p][t] << "\"];\n";

        // Post arcs (transition → place)
        for (size_t p = 0; p < P; p++)
            for (size_t t = 0; t < T; t++)
                if (model.Post[p][t] > 0)
                    out << "  \"" << model.transitions[t] << "\" -> \"" << model.places[p]
                        << "\" [label=\"" << model.Post[p][t] << "\"];\n";

        out << "}\n";
        out.close();
        cout << "[PARSER] DOT export completed" << endl;
    }

    // ==========================
    // 7. Validation và Logging
    // ==========================

    // Kiểm tra model có hợp lệ không
    if (!validateModel(model)) {
        throw runtime_error("Model validation failed");
    }

    cout << "[PARSER] PNML parsed successfully: "
         << "P=" << P << ", T=" << T << ", A=" << arcCount << endl;

    return model;
}

// ======== UTILITY FUNCTIONS ========

void printModelSummary(const Model& model) {
    cout << "=== PETRI NET MODEL SUMMARY ===" << endl;
    cout << "Places (" << model.places.size() << "): ";
    for (const auto& p : model.places) cout << p << " ";
    cout << endl;
    
    cout << "Transitions (" << model.transitions.size() << "): ";
    for (const auto& t : model.transitions) cout << t << " ";
    cout << endl;
    
    cout << "Initial Marking: " << toString(model.M0) << endl;
    cout << "Arcs: " << model.arcCount << endl;
    
    // Kiểm tra enabled transitions ở marking ban đầu
    vector<int> enabled;
    for (size_t t = 0; t < model.transitions.size(); ++t) {
        if (isEnabled(model, model.M0, t)) {
            enabled.push_back(t);
        }
    }
    cout << "Enabled transitions at M0: ";
    for (int t : enabled) cout << model.transitions[t] << " ";
    cout << endl;
    cout << "=================================" << endl;
}

bool validateModel(const Model& model) {
    // Kiểm tra cơ bản
    if (model.places.empty()) {
        cerr << "[VALIDATION] Error: No places found" << endl;
        return false;
    }
    
    if (model.M0.size() != model.places.size()) {
        cerr << "[VALIDATION] Error: M0 size doesn't match places count" << endl;
        return false;
    }
    
    // Kiểm tra 1-safe property cho M0 (cảnh báo)
    for (size_t i = 0; i < model.M0.size(); ++i) {
        if (model.M0[i] > 1) {
            cout << "[VALIDATION] Warning: Place " << model.places[i] 
                 << " has " << model.M0[i] << " tokens (may not be 1-safe)" << endl;
        }
    }
    
    cout << "[VALIDATION] Model validation passed" << endl;
    return true;
}
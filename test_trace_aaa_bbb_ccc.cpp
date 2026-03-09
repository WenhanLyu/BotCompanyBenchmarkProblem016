#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "bplustree.h"

using namespace std;

void analyzeResults(const string& key, const vector<int>& result, int expected) {
    cout << "\nAnalyzing key '" << key << "':" << endl;
    cout << "  Expected: " << expected << " values" << endl;
    cout << "  Found:    " << result.size() << " values" << endl;
    
    if (result.size() == expected) {
        cout << "  ✅ CORRECT" << endl;
        return;
    }
    
    cout << "  ❌ MISMATCH!" << endl;
    
    // Check for duplicates
    set<int> unique(result.begin(), result.end());
    if (unique.size() != result.size()) {
        cout << "  ⚠️ DUPLICATES DETECTED! Unique: " << unique.size() 
             << ", Total: " << result.size() << endl;
    }
    
    // Find missing values
    set<int> expected_set;
    for (int i = 0; i < expected; i++) expected_set.insert(i);
    
    vector<int> missing;
    for (int i = 0; i < expected; i++) {
        if (unique.find(i) == unique.end()) {
            missing.push_back(i);
        }
    }
    
    if (!missing.empty()) {
        cout << "  Missing " << missing.size() << " values:" << endl;
        
        // Show first 20
        cout << "    First 20: ";
        for (int i = 0; i < min(20, (int)missing.size()); i++) {
            cout << missing[i] << " ";
        }
        cout << endl;
        
        // Show last 20
        if (missing.size() > 20) {
            cout << "    Last 20:  ";
            for (int i = max(0, (int)missing.size() - 20); i < (int)missing.size(); i++) {
                cout << missing[i] << " ";
            }
            cout << endl;
        }
        
        // Show ranges
        if (!missing.empty()) {
            cout << "    Ranges:   ";
            int range_start = missing[0];
            int range_end = missing[0];
            for (size_t i = 1; i < missing.size(); i++) {
                if (missing[i] == range_end + 1) {
                    range_end = missing[i];
                } else {
                    if (range_start == range_end) {
                        cout << range_start << " ";
                    } else {
                        cout << range_start << "-" << range_end << " ";
                    }
                    range_start = missing[i];
                    range_end = missing[i];
                }
            }
            if (range_start == range_end) {
                cout << range_start;
            } else {
                cout << range_start << "-" << range_end;
            }
            cout << endl;
        }
    }
    
    // Find extra values
    vector<int> extra;
    for (int v : result) {
        if (v < 0 || v >= expected) {
            extra.push_back(v);
        }
    }
    
    if (!extra.empty()) {
        cout << "  Extra values (out of range): ";
        for (int i = 0; i < min(20, (int)extra.size()); i++) {
            cout << extra[i] << " ";
        }
        cout << endl;
    }
}

int main() {
    remove("test_aaa_bbb_ccc_trace.db");
    
    cout << "=== B+ Tree Debug Test: aaa/bbb/ccc with 996 values each ===" << endl;
    cout << "\nThis test will show all split operations and trace find() calls.\n" << endl;
    
    BPlusTree tree("test_aaa_bbb_ccc_trace.db");
    
    cout << "\n=== PHASE 1: Inserting 996 values for 'aaa' ===" << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("aaa", i);
    }
    cout << "Completed aaa insertions." << endl;
    
    cout << "\n=== PHASE 2: Inserting 996 values for 'bbb' ===" << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("bbb", i);
    }
    cout << "Completed bbb insertions." << endl;
    
    cout << "\n=== PHASE 3: Inserting 996 values for 'ccc' ===" << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("ccc", i);
    }
    cout << "Completed ccc insertions." << endl;
    
    cout << "\n=== PHASE 4: Finding all keys ===" << endl;
    
    vector<int> result_aaa = tree.find("aaa");
    analyzeResults("aaa", result_aaa, 996);
    
    vector<int> result_bbb = tree.find("bbb");
    analyzeResults("bbb", result_bbb, 996);
    
    vector<int> result_ccc = tree.find("ccc");
    analyzeResults("ccc", result_ccc, 996);
    
    cout << "\n=== SUMMARY ===" << endl;
    cout << "aaa: " << result_aaa.size() << "/996 " << (result_aaa.size() == 996 ? "✅" : "❌") << endl;
    cout << "bbb: " << result_bbb.size() << "/996 " << (result_bbb.size() == 996 ? "✅" : "❌") << endl;
    cout << "ccc: " << result_ccc.size() << "/996 " << (result_ccc.size() == 996 ? "✅" : "❌") << endl;
    
    remove("test_aaa_bbb_ccc_trace.db");
    return 0;
}

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include "bplustree.h"

using namespace std;

int main() {
    remove("test_aaa_bbb_ccc.db");
    
    BPlusTree tree("test_aaa_bbb_ccc.db");
    
    cout << "Inserting 996 values for 'aaa'..." << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("aaa", i);
    }
    
    cout << "Inserting 996 values for 'bbb'..." << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("bbb", i);
    }
    
    cout << "Inserting 996 values for 'ccc'..." << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("ccc", i);
    }
    
    cout << "\n=== FINDING ALL KEYS ===\n" << endl;
    
    cout << "Finding 'aaa'..." << endl;
    vector<int> result_aaa = tree.find("aaa");
    cout << "  Found " << result_aaa.size() << " values" << endl;
    if (result_aaa.size() != 996) {
        cout << "  ❌ ERROR: Expected 996, got " << result_aaa.size() << endl;
        cout << "  First 20 values: ";
        for (int i = 0; i < min(20, (int)result_aaa.size()); i++) {
            cout << result_aaa[i] << " ";
        }
        cout << endl;
        cout << "  Last 20 values: ";
        for (int i = max(0, (int)result_aaa.size() - 20); i < (int)result_aaa.size(); i++) {
            cout << result_aaa[i] << " ";
        }
        cout << endl;
    } else {
        cout << "  ✅ Correct count" << endl;
    }
    
    // Check for duplicates
    set<int> unique_aaa(result_aaa.begin(), result_aaa.end());
    if (unique_aaa.size() != result_aaa.size()) {
        cout << "  ⚠️ DUPLICATES in aaa! Unique: " << unique_aaa.size() << ", Total: " << result_aaa.size() << endl;
    }
    
    cout << "\nFinding 'bbb'..." << endl;
    vector<int> result_bbb = tree.find("bbb");
    cout << "  Found " << result_bbb.size() << " values" << endl;
    if (result_bbb.size() != 996) {
        cout << "  ❌ ERROR: Expected 996, got " << result_bbb.size() << endl;
        cout << "  First 20 values: ";
        for (int i = 0; i < min(20, (int)result_bbb.size()); i++) {
            cout << result_bbb[i] << " ";
        }
        cout << endl;
        cout << "  Last 20 values: ";
        for (int i = max(0, (int)result_bbb.size() - 20); i < (int)result_bbb.size(); i++) {
            cout << result_bbb[i] << " ";
        }
        cout << endl;
        
        // Find which values are missing
        set<int> expected;
        for (int i = 0; i < 996; i++) expected.insert(i);
        set<int> found(result_bbb.begin(), result_bbb.end());
        
        vector<int> missing;
        for (int i = 0; i < 996; i++) {
            if (found.find(i) == found.end()) {
                missing.push_back(i);
            }
        }
        
        if (!missing.empty()) {
            cout << "  Missing values (" << missing.size() << " total): ";
            for (int i = 0; i < min(50, (int)missing.size()); i++) {
                cout << missing[i] << " ";
            }
            if (missing.size() > 50) {
                cout << "... (showing first 50)";
            }
            cout << endl;
            
            // Find ranges
            if (!missing.empty()) {
                int range_start = missing[0];
                int range_end = missing[0];
                cout << "  Missing value ranges: ";
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
    } else {
        cout << "  ✅ Correct count" << endl;
    }
    
    // Check for duplicates
    set<int> unique_bbb(result_bbb.begin(), result_bbb.end());
    if (unique_bbb.size() != result_bbb.size()) {
        cout << "  ⚠️ DUPLICATES in bbb! Unique: " << unique_bbb.size() << ", Total: " << result_bbb.size() << endl;
    }
    
    cout << "\nFinding 'ccc'..." << endl;
    vector<int> result_ccc = tree.find("ccc");
    cout << "  Found " << result_ccc.size() << " values" << endl;
    if (result_ccc.size() != 996) {
        cout << "  ❌ ERROR: Expected 996, got " << result_ccc.size() << endl;
        cout << "  First 20 values: ";
        for (int i = 0; i < min(20, (int)result_ccc.size()); i++) {
            cout << result_ccc[i] << " ";
        }
        cout << endl;
        cout << "  Last 20 values: ";
        for (int i = max(0, (int)result_ccc.size() - 20); i < (int)result_ccc.size(); i++) {
            cout << result_ccc[i] << " ";
        }
        cout << endl;
    } else {
        cout << "  ✅ Correct count" << endl;
    }
    
    // Check for duplicates
    set<int> unique_ccc(result_ccc.begin(), result_ccc.end());
    if (unique_ccc.size() != result_ccc.size()) {
        cout << "  ⚠️ DUPLICATES in ccc! Unique: " << unique_ccc.size() << ", Total: " << result_ccc.size() << endl;
    }
    
    cout << "\n=== SUMMARY ===" << endl;
    cout << "aaa: " << result_aaa.size() << "/996" << (result_aaa.size() == 996 ? " ✅" : " ❌") << endl;
    cout << "bbb: " << result_bbb.size() << "/996" << (result_bbb.size() == 996 ? " ✅" : " ❌") << endl;
    cout << "ccc: " << result_ccc.size() << "/996" << (result_ccc.size() == 996 ? " ✅" : " ❌") << endl;
    
    remove("test_aaa_bbb_ccc.db");
    return 0;
}

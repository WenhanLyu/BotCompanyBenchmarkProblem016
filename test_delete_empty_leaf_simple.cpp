#include "bplustree.h"
#include <iostream>
#include <cstdio>
#include <algorithm>

using namespace std;

void cleanup(const string& db_name) {
    remove(db_name.c_str());
}

int main() {
    cout << "=== Simple Delete Empty Leaf Test ===" << endl;
    cleanup("test_simple_delete.db");
    
    BPlusTree tree("test_simple_delete.db");
    
    cout << "Step 1: Insert 1000 values (will cause leaf splits)..." << endl;
    for (int i = 0; i < 1000; i++) {
        tree.insert("key", i);
    }
    
    cout << "Step 2: Verify all 1000 values exist..." << endl;
    vector<int> before_delete = tree.find("key");
    cout << "Found " << before_delete.size() << " values before delete" << endl;
    
    if (before_delete.size() != 1000) {
        cout << "❌ FAIL: Expected 1000 values before delete, got " 
             << before_delete.size() << endl;
        cleanup("test_simple_delete.db");
        return 1;
    }
    
    cout << "Step 3: Delete first 500 values (0-499)..." << endl;
    for (int i = 0; i < 500; i++) {
        tree.remove("key", i);
    }
    
    cout << "Step 4: Find remaining values..." << endl;
    vector<int> after_delete = tree.find("key");
    cout << "Found " << after_delete.size() << " values after delete" << endl;
    
    if (after_delete.size() != 500) {
        cout << "❌ FAIL: Expected 500 values after delete, got " 
             << after_delete.size() << endl;
        
        // Print first 10 and last 10 values to debug
        cout << "First 10 values: ";
        for (int i = 0; i < min(10, (int)after_delete.size()); i++) {
            cout << after_delete[i] << " ";
        }
        cout << endl;
        
        if (after_delete.size() > 10) {
            cout << "Last 10 values: ";
            for (int i = max(0, (int)after_delete.size() - 10); 
                 i < (int)after_delete.size(); i++) {
                cout << after_delete[i] << " ";
            }
            cout << endl;
        }
        
        cleanup("test_simple_delete.db");
        return 1;
    }
    
    cout << "Step 5: Verify values are 500-999..." << endl;
    for (int i = 0; i < 500; i++) {
        if (after_delete[i] != i + 500) {
            cout << "❌ FAIL: Value mismatch at index " << i 
                 << ", expected " << (i + 500) 
                 << ", got " << after_delete[i] << endl;
            cleanup("test_simple_delete.db");
            return 1;
        }
    }
    
    cout << "✅ ALL CHECKS PASSED!" << endl;
    cleanup("test_simple_delete.db");
    return 0;
}

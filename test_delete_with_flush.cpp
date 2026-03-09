#include "bplustree.h"
#include <iostream>
#include <cstdio>

using namespace std;

void cleanup(const string& db_name) {
    remove(db_name.c_str());
}

int main() {
    cout << "=== Delete With Flush Test ===" << endl;
    cleanup("test_flush.db");
    
    BPlusTree tree("test_flush.db");
    
    // Insert 1000 values to force splits
    cout << "Inserting 1000 values..." << endl;
    for (int i = 0; i < 1000; i++) {
        tree.insert("key", i);
    }
    
    vector<int> before = tree.find("key");
    cout << "Before delete: " << before.size() << " values" << endl;
    
    // Delete with flush after each delete
    cout << "Deleting first 500 values (with flush)..." << endl;
    for (int i = 0; i < 500; i++) {
        tree.remove("key", i);
        if (i % 100 == 99) {
            tree.flush();
            cout << "  Flushed after " << (i+1) << " deletes" << endl;
        }
    }
    tree.flush();  // Final flush
    
    vector<int> after = tree.find("key");
    cout << "After delete: " << after.size() << " values" << endl;
    
    if (after.size() != 500) {
        cout << "❌ FAIL: Expected 500 values" << endl;
        
        // Show some values
        cout << "First 10: ";
        for (int i = 0; i < min(10, (int)after.size()); i++) {
            cout << after[i] << " ";
        }
        cout << endl;
    } else {
        cout << "✅ PASS" << endl;
    }
    
    cleanup("test_flush.db");
    return 0;
}

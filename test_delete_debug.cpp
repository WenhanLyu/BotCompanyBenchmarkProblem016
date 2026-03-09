#include "bplustree.h"
#include <iostream>
#include <cstdio>

using namespace std;

void cleanup(const string& db_name) {
    remove(db_name.c_str());
}

int main() {
    cout << "=== Debug Delete Test ===" << endl;
    cleanup("test_debug.db");
    
    BPlusTree tree("test_debug.db");
    
    // Insert just 10 values
    cout << "Inserting 10 values..." << endl;
    for (int i = 0; i < 10; i++) {
        tree.insert("key", i);
    }
    
    // Check they're there
    vector<int> before = tree.find("key");
    cout << "Before delete: " << before.size() << " values: ";
    for (int v : before) cout << v << " ";
    cout << endl;
    
    // Try to delete value 5
    cout << "\nAttempting to delete value 5..." << endl;
    bool removed = tree.remove("key", 5);
    cout << "Remove returned: " << (removed ? "true" : "false") << endl;
    
    // Check what's left
    vector<int> after = tree.find("key");
    cout << "After delete: " << after.size() << " values: ";
    for (int v : after) cout << v << " ";
    cout << endl;
    
    if (after.size() != 9) {
        cout << "❌ FAIL: Expected 9 values, got " << after.size() << endl;
    } else {
        cout << "✅ PASS: Correct count" << endl;
    }
    
    cleanup("test_debug.db");
    return 0;
}

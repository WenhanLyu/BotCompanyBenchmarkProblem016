#include "bplustree.h"
#include "file_manager.h"
#include "node.h"
#include <iostream>
#include <cstdio>

using namespace std;

void cleanup(const string& db_name) {
    remove(db_name.c_str());
}

// This test helps investigate why delete fails after leaf split
// by manually checking what findLeaf returns
int main() {
    cout << "=== Investigate findLeaf After Split ===" << endl;
    cleanup("test_investigate.db");
    
    BPlusTree tree("test_investigate.db");
    
    // Insert 1000 values to force split
    cout << "Inserting 1000 values..." << endl;
    for (int i = 0; i < 1000; i++) {
        tree.insert("key", i);
    }
    
    // Verify all values exist
    vector<int> all_values = tree.find("key");
    cout << "Total values found: " << all_values.size() << endl;
    cout << "First 10: ";
    for (int i = 0; i < 10; i++) cout << all_values[i] << " ";
    cout << endl;
    
    // Now let's trace what happens during a delete attempt
    cout << "\n=== Attempting to delete value 0 ===" << endl;
    
    // NOTE: Can't access FileManager internals from here
    // This test just shows the external behavior
    
    // Try the actual delete and see what happens
    bool deleted = tree.remove("key", 0);
    cout << "\nremove('key', 0) returned: " << (deleted ? "true" : "false") << endl;
    
    // Check if value 0 still exists
    vector<int> after_delete = tree.find("key");
    cout << "After delete attempt: " << after_delete.size() << " values" << endl;
    cout << "First value is now: " << after_delete[0] << endl;
    
    if (after_delete[0] == 0) {
        cout << "❌ Value 0 still exists - deletion failed!" << endl;
    } else {
        cout << "✅ Value 0 was deleted successfully" << endl;
    }
    
    cleanup("test_investigate.db");
    return 0;
}

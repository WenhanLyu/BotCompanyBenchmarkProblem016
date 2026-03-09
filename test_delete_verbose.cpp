#include "bplustree.h"
#include <iostream>
#include <cstdio>

using namespace std;

void cleanup(const string& db_name) {
    remove(db_name.c_str());
}

int main() {
    cout << "=== Verbose Delete Test ===" << endl;
    cout << "This test shows exactly where deletions start failing" << endl;
    cleanup("test_verbose.db");
    
    BPlusTree tree("test_verbose.db");
    
    // Test at different scales to see exact failure point
    int test_sizes[] = {996, 997, 998, 999, 1000};
    
    for (int size : test_sizes) {
        string db = "test_" + to_string(size) + ".db";
        cleanup(db);
        
        cout << "\n=== Test with " << size << " values ===" << endl;
        BPlusTree test_tree(db);
        
        // Insert
        for (int i = 0; i < size; i++) {
            test_tree.insert("key", i);
        }
        
        // Try to delete value 0
        bool result = test_tree.remove("key", 0);
        cout << "remove('key', 0) returned: " << (result ? "TRUE" : "FALSE") << endl;
        
        // Check if it's really gone
        vector<int> values = test_tree.find("key");
        cout << "Total values: " << values.size() << endl;
        cout << "First value: " << values[0] << endl;
        
        if (result && values[0] == 1) {
            cout << "✅ Delete worked correctly" << endl;
        } else if (!result && values[0] == 0) {
            cout << "❌ Delete failed - value still exists" << endl;
        } else {
            cout << "⚠️  Inconsistent state!" << endl;
        }
        
        cleanup(db);
    }
    
    cleanup("test_verbose.db");
    return 0;
}

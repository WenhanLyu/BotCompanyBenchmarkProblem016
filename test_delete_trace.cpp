#include "bplustree.h"
#include <iostream>
#include <cstdio>

using namespace std;

void cleanup(const string& db_name) {
    remove(db_name.c_str());
}

int main() {
    cout << "=== Delete Trace Test ===" << endl;
    cleanup("test_trace.db");
    
    BPlusTree tree("test_trace.db");
    
    // Insert 1000 values to force splits
    cout << "Inserting 1000 values..." << endl;
    for (int i = 0; i < 1000; i++) {
        tree.insert("key", i);
    }
    
    // Try to delete first 10 values and check after each
    cout << "\nDeleting values 0-9 one by one:" << endl;
    for (int i = 0; i < 10; i++) {
        bool removed = tree.remove("key", i);
        vector<int> current = tree.find("key");
        cout << "  Deleted " << i << " (returned " << removed << "), count=" 
             << current.size() << ", first value=" << current[0] << endl;
    }
    
    cleanup("test_trace.db");
    return 0;
}

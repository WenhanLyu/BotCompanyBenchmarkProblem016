#include "bplustree.h"
#include <iostream>
#include <cstdio>

using namespace std;

void cleanup(const string& db_name) {
    remove(db_name.c_str());
}

void test_delete_count(int insert_count, int delete_count) {
    string db_name = "test_" + to_string(insert_count) + ".db";
    cleanup(db_name);
    
    cout << "Testing: Insert " << insert_count << ", Delete " << delete_count << "..." << endl;
    
    BPlusTree tree(db_name);
    
    // Insert
    for (int i = 0; i < insert_count; i++) {
        tree.insert("key", i);
    }
    
    vector<int> before = tree.find("key");
    cout << "  Before: " << before.size() << " values" << endl;
    
    // Delete first delete_count values
    for (int i = 0; i < delete_count; i++) {
        tree.remove("key", i);
    }
    
    vector<int> after = tree.find("key");
    cout << "  After:  " << after.size() << " values" << endl;
    
    int expected = insert_count - delete_count;
    if (after.size() != expected) {
        cout << "  ❌ FAIL: Expected " << expected << " values" << endl;
    } else {
        cout << "  ✅ PASS" << endl;
    }
    
    cleanup(db_name);
}

int main() {
    cout << "=== Delete Scaling Test ===" << endl;
    
    test_delete_count(10, 5);
    test_delete_count(100, 50);
    test_delete_count(500, 250);
    test_delete_count(1000, 500);
    test_delete_count(2000, 1000);
    
    return 0;
}

#include "bplustree.h"
#include <iostream>
#include <cassert>
#include <cstdio>

using namespace std;

// Debug test to understand why key2 loses values
int main() {
    cout << "=== Debug Test for Issue #50 ===" << endl;
    
    remove("test_debug.db");
    
    BPlusTree tree("test_debug.db");
    
    // Insert key1 with 996 values
    cout << "Inserting key1 with 996 values..." << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("key1", i);
    }
    auto result1 = tree.find("key1");
    cout << "key1: " << result1.size() << " values" << endl;
    
    // Insert key2 with 996 values
    cout << "\nInserting key2 with 996 values..." << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("key2", i);
    }
    auto result2 = tree.find("key2");
    cout << "key2: " << result2.size() << " values" << endl;
    
    // Check key1 again
    result1 = tree.find("key1");
    cout << "key1 (recheck): " << result1.size() << " values" << endl;
    
    // Insert key3 with 996 values
    cout << "\nInserting key3 with 996 values..." << endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("key3", i);
    }
    auto result3 = tree.find("key3");
    cout << "key3: " << result3.size() << " values" << endl;
    
    // Recheck all keys
    cout << "\n=== Final Check ===" << endl;
    result1 = tree.find("key1");
    result2 = tree.find("key2");
    result3 = tree.find("key3");
    
    cout << "key1: " << result1.size() << " values" << (result1.size() == 996 ? "✅" : "❌") << endl;
    cout << "key2: " << result2.size() << " values" << (result2.size() == 996 ? "✅" : "❌") << endl;
    cout << "key3: " << result3.size() << " values" << (result3.size() == 996 ? "✅" : "❌") << endl;
    
    // Now test with a different insertion order
    cout << "\n=== Testing Reverse Order ===" << endl;
    remove("test_debug.db");
    BPlusTree tree2("test_debug.db");
    
    cout << "Inserting key3 with 996 values..." << endl;
    for (int i = 0; i < 996; i++) tree2.insert("key3", i);
    
    cout << "Inserting key2 with 996 values..." << endl;
    for (int i = 0; i < 996; i++) tree2.insert("key2", i);
    
    cout << "Inserting key1 with 996 values..." << endl;
    for (int i = 0; i < 996; i++) tree2.insert("key1", i);
    
    result1 = tree2.find("key1");
    result2 = tree2.find("key2");
    result3 = tree2.find("key3");
    
    cout << "\nFinal check:" << endl;
    cout << "key1: " << result1.size() << " values" << (result1.size() == 996 ? "✅" : "❌") << endl;
    cout << "key2: " << result2.size() << " values" << (result2.size() == 996 ? "✅" : "❌") << endl;
    cout << "key3: " << result3.size() << " values" << (result3.size() == 996 ? "✅" : "❌") << endl;
    
    remove("test_debug.db");
    
    return 0;
}

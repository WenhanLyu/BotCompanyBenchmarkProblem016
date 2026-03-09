#include <iostream>
#include <vector>
#include "bplustree.h"

int main() {
    // Clean start
    std::remove("debug_dup.db");
    
    BPlusTree tree("debug_dup.db");
    
    // Insert 3 values with same key
    std::cout << "Inserting 3 values with key 'test'...\n";
    tree.insert("test", 1);
    tree.insert("test", 2);
    tree.insert("test", 3);
    tree.flush();
    
    // Find the values
    std::cout << "Finding values for key 'test'...\n";
    std::vector<int> result = tree.find("test");
    
    std::cout << "Found " << result.size() << " values:\n";
    for (int v : result) {
        std::cout << v << "\n";
    }
    
    if (result.size() == 3) {
        std::cout << "\n✅ PASS: Found exactly 3 values\n";
        return 0;
    } else {
        std::cout << "\n❌ FAIL: Expected 3 values, got " << result.size() << "\n";
        return 1;
    }
}

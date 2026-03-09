#include <iostream>
#include <vector>
#include <set>
#include "bplustree.h"

// Custom find implementation with debug logging
std::vector<int> debug_find(BPlusTree& tree, const std::string& key) {
    std::cout << "\n=== DEBUG FIND FOR KEY: '" << key << "' ===\n";
    
    // We'll call the regular find but also manually trace
    std::vector<int> result = tree.find(key);
    
    std::cout << "Result size: " << result.size() << "\n";
    std::cout << "Values: ";
    for (int v : result) {
        std::cout << v << " ";
    }
    std::cout << "\n";
    
    // Check for duplicates
    std::set<int> unique(result.begin(), result.end());
    if (unique.size() != result.size()) {
        std::cout << "⚠️ DUPLICATES DETECTED! Unique: " << unique.size() 
                 << ", Total: " << result.size() << "\n";
    }
    
    return result;
}

int main() {
    std::remove("debug_trace.db");
    
    BPlusTree tree("debug_trace.db");
    
    // Test 1: Simple case with 3 values
    std::cout << "\n===== TEST 1: 3 values =====\n";
    tree.insert("test", 1);
    tree.insert("test", 2);
    tree.insert("test", 3);
    tree.flush();
    
    auto result1 = debug_find(tree, "test");
    
    // Test 2: More values to force multi-leaf
    std::cout << "\n\n===== TEST 2: 100 values =====\n";
    std::remove("debug_trace2.db");
    BPlusTree tree2("debug_trace2.db");
    
    for (int i = 1; i <= 100; i++) {
        tree2.insert("test", i);
    }
    tree2.flush();
    
    auto result2 = debug_find(tree2, "test");
    
    // Test 3: Check if caching causes issues - query twice
    std::cout << "\n\n===== TEST 3: Query twice (cache test) =====\n";
    std::cout << "First query:\n";
    auto result3a = debug_find(tree2, "test");
    
    std::cout << "\nSecond query (should hit cache):\n";
    auto result3b = debug_find(tree2, "test");
    
    if (result3a.size() == result3b.size()) {
        std::cout << "✅ Both queries returned same count\n";
    } else {
        std::cout << "❌ Query counts differ! First: " << result3a.size() 
                 << ", Second: " << result3b.size() << "\n";
    }
    
    return 0;
}

#include <iostream>
#include <vector>
#include <set>
#include "bplustree.h"

int main() {
    // Test with different value counts
    std::vector<int> test_counts = {3, 10, 100, 1000};
    
    for (int count : test_counts) {
        std::string db_name = "debug_ml_" + std::to_string(count) + ".db";
        std::remove(db_name.c_str());
        
        BPlusTree tree(db_name);
        
        std::cout << "\n=== Testing with " << count << " values ===\n";
        
        // Insert values
        std::cout << "Inserting " << count << " values with key 'test'...\n";
        for (int i = 1; i <= count; i++) {
            tree.insert("test", i);
        }
        tree.flush();
        
        // Find the values
        std::cout << "Finding values for key 'test'...\n";
        std::vector<int> result = tree.find("test");
        
        std::cout << "Expected: " << count << " values\n";
        std::cout << "Found:    " << result.size() << " values\n";
        
        // Check for duplicates
        std::set<int> unique_values(result.begin(), result.end());
        if (unique_values.size() != result.size()) {
            std::cout << "⚠️  WARNING: Found " << (result.size() - unique_values.size()) 
                     << " duplicate values!\n";
        }
        
        // Check if all values are present
        bool all_present = true;
        for (int i = 1; i <= count; i++) {
            if (unique_values.find(i) == unique_values.end()) {
                std::cout << "❌ Missing value: " << i << "\n";
                all_present = false;
            }
        }
        
        if (result.size() == count && all_present) {
            std::cout << "✅ PASS: Found exactly " << count << " unique values\n";
        } else {
            std::cout << "❌ FAIL: Expected " << count << " values, got " << result.size() 
                     << " (" << unique_values.size() << " unique)\n";
            
            // Show first few duplicates if any
            if (result.size() > count && result.size() <= 20) {
                std::cout << "All values: ";
                for (int v : result) {
                    std::cout << v << " ";
                }
                std::cout << "\n";
            }
        }
    }
    
    return 0;
}

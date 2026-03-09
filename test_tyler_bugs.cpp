#include "bplustree.h"
#include <iostream>
#include <cassert>

void test_bug1_duplicate_insertion() {
    std::cout << "\n=== Testing Bug #1: Duplicate Insertion After Split ===" << std::endl;
    
    // Remove any existing database
    std::remove("test_bug1.db");
    
    BPlusTree tree("test_bug1.db");
    
    // Insert 996 values for key "A" - should trigger value-level split
    std::cout << "Inserting 996 values for key A..." << std::endl;
    for (int i = 1; i <= 996; i++) {
        tree.insert("A", i);
    }
    
    // Verify 996 values exist
    std::vector<int> values = tree.find("A");
    std::cout << "After initial insert: found " << values.size() << " values" << std::endl;
    assert(values.size() == 996);
    
    // Try to re-insert value 100 (which already exists)
    std::cout << "Re-inserting value 100 (should be ignored)..." << std::endl;
    tree.insert("A", 100);
    
    // Check if duplicate was created
    values = tree.find("A");
    std::cout << "After re-insert: found " << values.size() << " values" << std::endl;
    
    if (values.size() == 996) {
        std::cout << "✅ Bug #1 FIXED: No duplicate created" << std::endl;
    } else {
        std::cout << "❌ Bug #1 EXISTS: Duplicate created! Expected 996, got " << values.size() << std::endl;
        assert(false);
    }
    
    std::remove("test_bug1.db");
}

void test_bug2_find_empty_starting_leaf() {
    std::cout << "\n=== Testing Bug #2: find() Early Return on Empty Starting Leaf ===" << std::endl;
    
    // Remove any existing database
    std::remove("test_bug2.db");
    
    BPlusTree tree("test_bug2.db");
    
    // Insert 996 values for key "A" - should trigger value-level split
    // This creates two leaves: left [1..497] and right [498..996]
    std::cout << "Inserting 996 values for key A..." << std::endl;
    for (int i = 1; i <= 996; i++) {
        tree.insert("A", i);
    }
    
    // Verify 996 values exist
    std::vector<int> values = tree.find("A");
    std::cout << "After initial insert: found " << values.size() << " values" << std::endl;
    assert(values.size() == 996);
    
    // Delete all values from the right leaf (498-996)
    std::cout << "Deleting values 498-996..." << std::endl;
    for (int i = 498; i <= 996; i++) {
        tree.remove("A", i);
    }
    
    // Now find("A") should still return values from left leaf [1..497]
    // But if Bug #2 exists, findLeaf() returns right leaf (now empty), and find() returns early
    values = tree.find("A");
    std::cout << "After deleting right leaf: found " << values.size() << " values" << std::endl;
    
    if (values.size() == 497) {
        std::cout << "✅ Bug #2 FIXED: Found all remaining values" << std::endl;
    } else {
        std::cout << "❌ Bug #2 EXISTS: Early return prevented finding values! Expected 497, got " << values.size() << std::endl;
        assert(false);
    }
    
    std::remove("test_bug2.db");
}

int main() {
    try {
        test_bug1_duplicate_insertion();
        test_bug2_find_empty_starting_leaf();
        
        std::cout << "\n✅ ALL TESTS PASSED!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

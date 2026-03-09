#include "bplustree.h"
#include <iostream>
#include <cassert>
#include <cstdio>
#include <algorithm>

using namespace std;

// Utility to clean up test databases
void cleanup(const string& db_name) {
    remove(db_name.c_str());
}

// TEST 1: Basic Delete+Find
// Insert values, delete some, verify find returns correct remaining values
void test_basic_delete_find() {
    cout << "Test 1: Basic Delete+Find... " << flush;
    cleanup("test_delete1.db");
    
    BPlusTree tree("test_delete1.db");
    
    // Insert 100 values
    for (int i = 0; i < 100; i++) {
        tree.insert("testkey", i);
    }
    
    // Delete first 50 values
    for (int i = 0; i < 50; i++) {
        tree.remove("testkey", i);
    }
    
    // Find should return remaining 50 values (50-99)
    vector<int> result = tree.find("testkey");
    
    if (result.size() != 50) {
        cout << "❌ FAIL: expected 50 values, got " << result.size() << endl;
        cleanup("test_delete1.db");
        exit(1);
    }
    
    // Verify values are correct and sorted
    for (int i = 0; i < 50; i++) {
        if (result[i] != i + 50) {
            cout << "❌ FAIL: value mismatch at index " << i 
                 << ", expected " << (i + 50) << ", got " << result[i] << endl;
            cleanup("test_delete1.db");
            exit(1);
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test_delete1.db");
}

// TEST 2: Delete Creates Empty Leaf
// Insert enough values to cause leaf split, then delete ALL values from one leaf
void test_delete_creates_empty_leaf() {
    cout << "Test 2: Delete Creates Empty Leaf... " << flush;
    cleanup("test_delete2.db");
    
    BPlusTree tree("test_delete2.db");
    
    // Insert 1000 values to force leaf splits
    for (int i = 0; i < 1000; i++) {
        tree.insert("key", i);
    }
    
    // Delete first 500 values (likely to empty at least one leaf)
    for (int i = 0; i < 500; i++) {
        tree.remove("key", i);
    }
    
    // Find should return remaining 500 values
    vector<int> result = tree.find("key");
    
    if (result.size() != 500) {
        cout << "❌ FAIL: expected 500 values, got " << result.size() << endl;
        cleanup("test_delete2.db");
        exit(1);
    }
    
    // Verify values are 500-999
    for (int i = 0; i < 500; i++) {
        if (result[i] != i + 500) {
            cout << "❌ FAIL: value mismatch at index " << i << endl;
            cleanup("test_delete2.db");
            exit(1);
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test_delete2.db");
}

// TEST 3: Delete+Find Across Persistence
// Insert, save, delete, save, reload, find
void test_delete_find_persistence() {
    cout << "Test 3: Delete+Find Across Persistence... " << flush;
    cleanup("test_delete3.db");
    
    // Phase 1: Insert and save
    {
        BPlusTree tree("test_delete3.db");
        for (int i = 0; i < 200; i++) {
            tree.insert("persist", i);
        }
        tree.flush();
    }
    
    // Phase 2: Reload, delete, and save
    {
        BPlusTree tree("test_delete3.db");
        vector<int> before_delete = tree.find("persist");
        if (before_delete.size() != 200) {
            cout << "❌ FAIL: before delete, expected 200 values, got " 
                 << before_delete.size() << endl;
            cleanup("test_delete3.db");
            exit(1);
        }
        
        // Delete first 100 values
        for (int i = 0; i < 100; i++) {
            tree.remove("persist", i);
        }
        tree.flush();
    }
    
    // Phase 3: Reload and verify
    {
        BPlusTree tree("test_delete3.db");
        vector<int> result = tree.find("persist");
        
        if (result.size() != 100) {
            cout << "❌ FAIL: after persistence, expected 100 values, got " 
                 << result.size() << endl;
            cleanup("test_delete3.db");
            exit(1);
        }
        
        // Verify values are 100-199
        for (int i = 0; i < 100; i++) {
            if (result[i] != i + 100) {
                cout << "❌ FAIL: value mismatch after persistence at index " << i << endl;
                cleanup("test_delete3.db");
                exit(1);
            }
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test_delete3.db");
}

// TEST 4: Multi-Key Delete Scenarios
// Delete values from middle key, ensure other keys unaffected
void test_multikey_delete() {
    cout << "Test 4: Multi-Key Delete Scenarios... " << flush;
    cleanup("test_delete4.db");
    
    BPlusTree tree("test_delete4.db");
    
    // Insert values for three keys
    for (int i = 0; i < 500; i++) tree.insert("key1", i);
    for (int i = 0; i < 500; i++) tree.insert("key2", i);
    for (int i = 0; i < 500; i++) tree.insert("key3", i);
    
    // Delete all values from middle key (key2)
    for (int i = 0; i < 500; i++) {
        tree.remove("key2", i);
    }
    
    // Verify key1 still has all values
    vector<int> result1 = tree.find("key1");
    if (result1.size() != 500) {
        cout << "❌ FAIL: key1 expected 500 values, got " << result1.size() << endl;
        cleanup("test_delete4.db");
        exit(1);
    }
    
    // Verify key2 is empty
    vector<int> result2 = tree.find("key2");
    if (!result2.empty()) {
        cout << "❌ FAIL: key2 expected 0 values, got " << result2.size() << endl;
        cleanup("test_delete4.db");
        exit(1);
    }
    
    // Verify key3 still has all values
    vector<int> result3 = tree.find("key3");
    if (result3.size() != 500) {
        cout << "❌ FAIL: key3 expected 500 values, got " << result3.size() << endl;
        cleanup("test_delete4.db");
        exit(1);
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test_delete4.db");
}

// TEST 5: Delete from First/Middle/Last Leaves
// Create multiple leaves, delete from different positions
void test_delete_from_leaf_positions() {
    cout << "Test 5: Delete from First/Middle/Last Leaves... " << flush;
    cleanup("test_delete5.db");
    
    BPlusTree tree("test_delete5.db");
    
    // Insert 2000 values to create multiple leaves
    for (int i = 0; i < 2000; i++) {
        tree.insert("multiLeaf", i);
    }
    
    // Delete from beginning (first leaf)
    for (int i = 0; i < 200; i++) {
        tree.remove("multiLeaf", i);
    }
    
    // Delete from middle (around value 1000)
    for (int i = 900; i < 1100; i++) {
        tree.remove("multiLeaf", i);
    }
    
    // Delete from end (last leaf)
    for (int i = 1800; i < 2000; i++) {
        tree.remove("multiLeaf", i);
    }
    
    // Should have deleted 200 + 200 + 200 = 600 values
    // Remaining: 2000 - 600 = 1400
    vector<int> result = tree.find("multiLeaf");
    
    if (result.size() != 1400) {
        cout << "❌ FAIL: expected 1400 values, got " << result.size() << endl;
        cleanup("test_delete5.db");
        exit(1);
    }
    
    // Verify values are sorted
    for (size_t i = 1; i < result.size(); i++) {
        if (result[i] <= result[i-1]) {
            cout << "❌ FAIL: values not sorted" << endl;
            cleanup("test_delete5.db");
            exit(1);
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test_delete5.db");
}

// TEST 6: Delete Non-Existent Values
// Ensure deleting non-existent values doesn't break find
void test_delete_nonexistent() {
    cout << "Test 6: Delete Non-Existent Values... " << flush;
    cleanup("test_delete6.db");
    
    BPlusTree tree("test_delete6.db");
    
    // Insert values 0-99
    for (int i = 0; i < 100; i++) {
        tree.insert("key", i);
    }
    
    // Try to delete values that don't exist (100-199)
    for (int i = 100; i < 200; i++) {
        tree.remove("key", i);
    }
    
    // Original values should still be there
    vector<int> result = tree.find("key");
    
    if (result.size() != 100) {
        cout << "❌ FAIL: expected 100 values, got " << result.size() << endl;
        cleanup("test_delete6.db");
        exit(1);
    }
    
    // Verify values are 0-99
    for (int i = 0; i < 100; i++) {
        if (result[i] != i) {
            cout << "❌ FAIL: value mismatch at index " << i << endl;
            cleanup("test_delete6.db");
            exit(1);
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test_delete6.db");
}

// TEST 7: Delete All Then Re-Insert
// Delete all values from a key, then insert new values
void test_delete_all_reinsert() {
    cout << "Test 7: Delete All Then Re-Insert... " << flush;
    cleanup("test_delete7.db");
    
    BPlusTree tree("test_delete7.db");
    
    // Insert initial values
    for (int i = 0; i < 500; i++) {
        tree.insert("reinsert", i);
    }
    
    // Delete all values
    for (int i = 0; i < 500; i++) {
        tree.remove("reinsert", i);
    }
    
    // Verify key is empty
    vector<int> empty_result = tree.find("reinsert");
    if (!empty_result.empty()) {
        cout << "❌ FAIL: after delete all, expected 0 values, got " 
             << empty_result.size() << endl;
        cleanup("test_delete7.db");
        exit(1);
    }
    
    // Re-insert different values
    for (int i = 1000; i < 1200; i++) {
        tree.insert("reinsert", i);
    }
    
    // Verify new values
    vector<int> result = tree.find("reinsert");
    if (result.size() != 200) {
        cout << "❌ FAIL: after re-insert, expected 200 values, got " 
             << result.size() << endl;
        cleanup("test_delete7.db");
        exit(1);
    }
    
    // Verify values are 1000-1199
    for (int i = 0; i < 200; i++) {
        if (result[i] != i + 1000) {
            cout << "❌ FAIL: re-inserted value mismatch at index " << i << endl;
            cleanup("test_delete7.db");
            exit(1);
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test_delete7.db");
}

// TEST 8: Alternating Delete Pattern
// Delete every other value, check find returns correct remaining values
void test_alternating_delete() {
    cout << "Test 8: Alternating Delete Pattern... " << flush;
    cleanup("test_delete8.db");
    
    BPlusTree tree("test_delete8.db");
    
    // Insert 1000 values
    for (int i = 0; i < 1000; i++) {
        tree.insert("alternate", i);
    }
    
    // Delete every other value (even numbers)
    for (int i = 0; i < 1000; i += 2) {
        tree.remove("alternate", i);
    }
    
    // Should have 500 odd numbers remaining
    vector<int> result = tree.find("alternate");
    
    if (result.size() != 500) {
        cout << "❌ FAIL: expected 500 values, got " << result.size() << endl;
        cleanup("test_delete8.db");
        exit(1);
    }
    
    // Verify all values are odd numbers
    for (int i = 0; i < 500; i++) {
        if (result[i] != 2*i + 1) {
            cout << "❌ FAIL: expected odd value " << (2*i + 1) 
                 << ", got " << result[i] << " at index " << i << endl;
            cleanup("test_delete8.db");
            exit(1);
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test_delete8.db");
}

int main() {
    cout << "=== Comprehensive Delete+Find Test Suite ===" << endl;
    cout << "Testing delete operations followed by find operations" << endl;
    cout << "Focus: scenarios that create empty leaves and chain traversal" << endl;
    cout << endl;
    
    test_basic_delete_find();              // Test 1
    test_delete_creates_empty_leaf();      // Test 2
    test_delete_find_persistence();        // Test 3
    test_multikey_delete();                // Test 4
    test_delete_from_leaf_positions();     // Test 5
    test_delete_nonexistent();             // Test 6
    test_delete_all_reinsert();            // Test 7
    test_alternating_delete();             // Test 8
    
    cout << endl;
    cout << "=== All delete+find tests passed! ===" << endl;
    return 0;
}

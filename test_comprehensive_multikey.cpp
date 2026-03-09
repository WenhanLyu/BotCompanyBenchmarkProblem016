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

// Test 1: Basic Multi-Key (Tara's Case)
void test_basic_multikey() {
    cout << "Test 1: Basic Multi-Key (small/medium/large)... " << flush;
    cleanup("test1.db");
    
    BPlusTree tree("test1.db");
    tree.insert("small", 1);
    for (int i = 0; i < 100; i++) tree.insert("medium", i);
    for (int i = 0; i < 996; i++) tree.insert("large", i);
    
    vector<int> small_result = tree.find("small");
    vector<int> medium_result = tree.find("medium");
    vector<int> large_result = tree.find("large");
    
    if (small_result.size() != 1) {
        cout << "❌ FAIL: small key expected 1 value, got " << small_result.size() << endl;
        cleanup("test1.db");
        exit(1);
    }
    if (medium_result.size() != 100) {
        cout << "❌ FAIL: medium key expected 100 values, got " << medium_result.size() << endl;
        cleanup("test1.db");
        exit(1);
    }
    if (large_result.size() != 996) {
        cout << "❌ FAIL: large key expected 996 values, got " << large_result.size() << endl;
        cleanup("test1.db");
        exit(1);
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test1.db");
}

// Test 2: Alphabetical Order Variations
void test_alphabetical_order() {
    cout << "Test 2: Alphabetical Order Variations... " << flush;
    
    // Test 2a: Insert a, b, c (alphabetical)
    cleanup("test2a.db");
    {
        BPlusTree tree("test2a.db");
        for (int i = 0; i < 996; i++) tree.insert("aaa", i);
        for (int i = 0; i < 996; i++) tree.insert("bbb", i);
        for (int i = 0; i < 996; i++) tree.insert("ccc", i);
        
        if (tree.find("aaa").size() != 996 || 
            tree.find("bbb").size() != 996 || 
            tree.find("ccc").size() != 996) {
            cout << "❌ FAIL: alphabetical order - aaa=" << tree.find("aaa").size() 
                 << " bbb=" << tree.find("bbb").size() 
                 << " ccc=" << tree.find("ccc").size() << endl;
            cleanup("test2a.db");
            exit(1);
        }
    }
    cleanup("test2a.db");
    
    // Test 2b: Insert c, b, a (reverse)
    cleanup("test2b.db");
    {
        BPlusTree tree("test2b.db");
        for (int i = 0; i < 996; i++) tree.insert("ccc", i);
        for (int i = 0; i < 996; i++) tree.insert("bbb", i);
        for (int i = 0; i < 996; i++) tree.insert("aaa", i);
        
        if (tree.find("aaa").size() != 996 || 
            tree.find("bbb").size() != 996 || 
            tree.find("ccc").size() != 996) {
            cout << "❌ FAIL: reverse order - aaa=" << tree.find("aaa").size() 
                 << " bbb=" << tree.find("bbb").size() 
                 << " ccc=" << tree.find("ccc").size() << endl;
            cleanup("test2b.db");
            exit(1);
        }
    }
    cleanup("test2b.db");
    
    // Test 2c: Insert b, a, c (mixed)
    cleanup("test2c.db");
    {
        BPlusTree tree("test2c.db");
        for (int i = 0; i < 996; i++) tree.insert("bbb", i);
        for (int i = 0; i < 996; i++) tree.insert("aaa", i);
        for (int i = 0; i < 996; i++) tree.insert("ccc", i);
        
        if (tree.find("aaa").size() != 996 || 
            tree.find("bbb").size() != 996 || 
            tree.find("ccc").size() != 996) {
            cout << "❌ FAIL: mixed order - aaa=" << tree.find("aaa").size() 
                 << " bbb=" << tree.find("bbb").size() 
                 << " ccc=" << tree.find("ccc").size() << endl;
            cleanup("test2c.db");
            exit(1);
        }
    }
    cleanup("test2c.db");
    
    cout << "✅ PASS" << endl;
}

// Test 3: Multiple Keys All Split (5 keys, each 996+ values)
void test_multiple_keys_all_split() {
    cout << "Test 3: Multiple Keys All Split (5 keys x 996 values)... " << flush;
    cleanup("test3.db");
    
    BPlusTree tree("test3.db");
    for (int i = 0; i < 996; i++) tree.insert("key1", i);
    for (int i = 0; i < 996; i++) tree.insert("key2", i);
    for (int i = 0; i < 996; i++) tree.insert("key3", i);
    for (int i = 0; i < 996; i++) tree.insert("key4", i);
    for (int i = 0; i < 996; i++) tree.insert("key5", i);
    
    vector<int> r1 = tree.find("key1");
    vector<int> r2 = tree.find("key2");
    vector<int> r3 = tree.find("key3");
    vector<int> r4 = tree.find("key4");
    vector<int> r5 = tree.find("key5");
    
    if (r1.size() != 996 || r2.size() != 996 || r3.size() != 996 || 
        r4.size() != 996 || r5.size() != 996) {
        cout << "❌ FAIL: key1=" << r1.size() << " key2=" << r2.size() 
             << " key3=" << r3.size() << " key4=" << r4.size() 
             << " key5=" << r5.size() << endl;
        cleanup("test3.db");
        exit(1);
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test3.db");
}

// Test 4: Edge Case - Keys That Compare Equal to Split Keys
void test_split_key_comparisons() {
    cout << "Test 4: Split Key Comparison Edge Cases... " << flush;
    cleanup("test4.db");
    
    BPlusTree tree("test4.db");
    // Insert keys that will create split keys in internal nodes
    for (int i = 0; i < 996; i++) tree.insert("aaa", i);
    for (int i = 0; i < 996; i++) tree.insert("aab", i);
    for (int i = 0; i < 996; i++) tree.insert("aac", i);
    
    vector<int> r1 = tree.find("aaa");
    vector<int> r2 = tree.find("aab");
    vector<int> r3 = tree.find("aac");
    
    if (r1.size() != 996 || r2.size() != 996 || r3.size() != 996) {
        cout << "❌ FAIL: aaa=" << r1.size() << " aab=" << r2.size() 
             << " aac=" << r3.size() << endl;
        cleanup("test4.db");
        exit(1);
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test4.db");
}

// Test 5: Mixed Operations (insert, delete, insert more)
void test_mixed_operations() {
    cout << "Test 5: Mixed Operations (insert/delete/insert)... " << flush;
    cleanup("test5.db");
    
    BPlusTree tree("test5.db");
    // Insert multi-key data
    for (int i = 0; i < 996; i++) tree.insert("key1", i);
    for (int i = 0; i < 996; i++) tree.insert("key2", i);
    
    // Delete some values from key1
    for (int i = 0; i < 100; i++) tree.remove("key1", i);
    
    // Insert more to key2
    for (int i = 996; i < 1100; i++) tree.insert("key2", i);
    
    vector<int> r1 = tree.find("key1");
    vector<int> r2 = tree.find("key2");
    
    if (r1.size() != 896) {  // 996 - 100
        cout << "❌ FAIL: key1 expected 896 values, got " << r1.size() << endl;
        cleanup("test5.db");
        exit(1);
    }
    if (r2.size() != 1100) {
        cout << "❌ FAIL: key2 expected 1100 values, got " << r2.size() << endl;
        cleanup("test5.db");
        exit(1);
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test5.db");
}

// Test 6: Boundary Cases
void test_boundary_cases() {
    cout << "Test 6: Boundary Cases... " << flush;
    
    // Test 6a: Empty tree find
    cleanup("test6a.db");
    {
        BPlusTree tree("test6a.db");
        vector<int> result = tree.find("nonexistent");
        if (!result.empty()) {
            cout << "❌ FAIL: empty tree find returned non-empty" << endl;
            cleanup("test6a.db");
            exit(1);
        }
    }
    cleanup("test6a.db");
    
    // Test 6b: Single key with 0-1000 values
    cleanup("test6b.db");
    {
        BPlusTree tree("test6b.db");
        for (int i = 0; i < 1000; i++) tree.insert("key", i);
        vector<int> result = tree.find("key");
        if (result.size() != 1000) {
            cout << "❌ FAIL: single key 1000 values, got " << result.size() << endl;
            cleanup("test6b.db");
            exit(1);
        }
    }
    cleanup("test6b.db");
    
    // Test 6c: Two keys with different value counts
    cleanup("test6c.db");
    {
        BPlusTree tree("test6c.db");
        for (int i = 0; i < 995; i++) tree.insert("key1", i);
        for (int i = 0; i < 997; i++) tree.insert("key2", i);
        
        if (tree.find("key1").size() != 995 || tree.find("key2").size() != 997) {
            cout << "❌ FAIL: key1=" << tree.find("key1").size() 
                 << " key2=" << tree.find("key2").size() << endl;
            cleanup("test6c.db");
            exit(1);
        }
    }
    cleanup("test6c.db");
    
    // Test 6d: 10 keys each with 100 values
    cleanup("test6d.db");
    {
        BPlusTree tree("test6d.db");
        for (int k = 0; k < 10; k++) {
            string key = "key" + to_string(k);
            for (int i = 0; i < 100; i++) {
                tree.insert(key, i);
            }
        }
        
        for (int k = 0; k < 10; k++) {
            string key = "key" + to_string(k);
            if (tree.find(key).size() != 100) {
                cout << "❌ FAIL: " << key << " expected 100 values, got " 
                     << tree.find(key).size() << endl;
                cleanup("test6d.db");
                exit(1);
            }
        }
    }
    cleanup("test6d.db");
    
    cout << "✅ PASS" << endl;
}

// Test 7: Exact 996 boundary (the critical case)
void test_996_boundary() {
    cout << "Test 7: Exact 996 Boundary (critical case)... " << flush;
    cleanup("test7.db");
    
    BPlusTree tree("test7.db");
    for (int i = 0; i < 996; i++) tree.insert("testkey", i);
    
    vector<int> result = tree.find("testkey");
    if (result.size() != 996) {
        cout << "❌ FAIL: expected 996 values, got " << result.size() << endl;
        cleanup("test7.db");
        exit(1);
    }
    
    // Verify all values are present and sorted
    vector<int> expected(996);
    for (int i = 0; i < 996; i++) expected[i] = i;
    
    if (result != expected) {
        cout << "❌ FAIL: values not correct or not sorted" << endl;
        cleanup("test7.db");
        exit(1);
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test7.db");
}

// Test 8: Beyond 996 boundary (997, 1000, 2000)
void test_beyond_996() {
    cout << "Test 8: Beyond 996 Boundary (997, 1000, 2000)... " << flush;
    
    // Test 997 values
    cleanup("test8a.db");
    {
        BPlusTree tree("test8a.db");
        for (int i = 0; i < 997; i++) tree.insert("key997", i);
        if (tree.find("key997").size() != 997) {
            cout << "❌ FAIL: 997 values, got " << tree.find("key997").size() << endl;
            cleanup("test8a.db");
            exit(1);
        }
    }
    cleanup("test8a.db");
    
    // Test 1000 values
    cleanup("test8b.db");
    {
        BPlusTree tree("test8b.db");
        for (int i = 0; i < 1000; i++) tree.insert("key1000", i);
        if (tree.find("key1000").size() != 1000) {
            cout << "❌ FAIL: 1000 values, got " << tree.find("key1000").size() << endl;
            cleanup("test8b.db");
            exit(1);
        }
    }
    cleanup("test8b.db");
    
    // Test 2000 values
    cleanup("test8c.db");
    {
        BPlusTree tree("test8c.db");
        for (int i = 0; i < 2000; i++) tree.insert("key2000", i);
        if (tree.find("key2000").size() != 2000) {
            cout << "❌ FAIL: 2000 values, got " << tree.find("key2000").size() << endl;
            cleanup("test8c.db");
            exit(1);
        }
    }
    cleanup("test8c.db");
    
    cout << "✅ PASS" << endl;
}

// Test 9: Internal routing with equal keys (tests the <= fix)
void test_internal_routing() {
    cout << "Test 9: Internal Routing with Equal Keys... " << flush;
    cleanup("test9.db");
    
    BPlusTree tree("test9.db");
    // Insert enough data to create internal nodes with split keys
    for (int i = 0; i < 1500; i++) tree.insert("equalkey", i);
    
    vector<int> result = tree.find("equalkey");
    if (result.size() != 1500) {
        cout << "❌ FAIL: expected 1500 values, got " << result.size() << endl;
        cleanup("test9.db");
        exit(1);
    }
    
    // Verify values are correct and sorted
    for (int i = 0; i < 1500; i++) {
        if (result[i] != i) {
            cout << "❌ FAIL: value mismatch at index " << i << endl;
            cleanup("test9.db");
            exit(1);
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test9.db");
}

// Test 10: Stress test - many keys, varying value counts
void test_stress() {
    cout << "Test 10: Stress Test (20 keys, varying counts)... " << flush;
    cleanup("test10.db");
    
    BPlusTree tree("test10.db");
    int counts[] = {1, 10, 50, 100, 500, 994, 995, 996, 997, 998, 
                    1000, 1100, 1200, 1500, 2000, 2500, 3000, 100, 50, 1};
    
    for (int k = 0; k < 20; k++) {
        string key = "stress" + to_string(k);
        for (int i = 0; i < counts[k]; i++) {
            tree.insert(key, i);
        }
    }
    
    for (int k = 0; k < 20; k++) {
        string key = "stress" + to_string(k);
        vector<int> result = tree.find(key);
        if (result.size() != (size_t)counts[k]) {
            cout << "❌ FAIL: " << key << " expected " << counts[k] 
                 << " values, got " << result.size() << endl;
            cleanup("test10.db");
            exit(1);
        }
    }
    
    cout << "✅ PASS" << endl;
    cleanup("test10.db");
}

int main() {
    cout << "=== Comprehensive Multi-Key Test Suite ===" << endl;
    cout << "Testing commit bcf348d (single-entry split fix)" << endl;
    cout << endl;
    
    test_basic_multikey();              // Test 1
    test_alphabetical_order();          // Test 2
    test_multiple_keys_all_split();     // Test 3
    test_split_key_comparisons();       // Test 4
    test_mixed_operations();            // Test 5
    test_boundary_cases();              // Test 6
    test_996_boundary();                // Test 7
    test_beyond_996();                  // Test 8
    test_internal_routing();            // Test 9
    test_stress();                      // Test 10
    
    cout << endl;
    cout << "=== All tests passed! ===" << endl;
    return 0;
}

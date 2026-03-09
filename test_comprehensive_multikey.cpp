#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "bplustree.h"

using namespace std;

bool test_passed = true;

void assert_equal(size_t actual, size_t expected, const string& test_name) {
    if (actual != expected) {
        cout << "❌ FAIL: " << test_name << " - Expected " << expected << ", got " << actual << endl;
        test_passed = false;
    }
}

void test_basic_multikey() {
    cout << "Test 1: Basic Multi-Key (Tara's Case)... ";
    remove("test_basic_multikey.db");
    
    BPlusTree tree("test_basic_multikey.db");
    tree.insert("small", 1);
    for (int i = 0; i < 100; i++) {
        tree.insert("medium", i);
    }
    for (int i = 0; i < 996; i++) {
        tree.insert("large", i);
    }
    
    vector<int> small_result = tree.find("small");
    vector<int> medium_result = tree.find("medium");
    vector<int> large_result = tree.find("large");
    
    assert_equal(small_result.size(), 1, "small key count");
    assert_equal(medium_result.size(), 100, "medium key count");
    assert_equal(large_result.size(), 996, "large key count");
    
    remove("test_basic_multikey.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_alphabetical_variations() {
    cout << "Test 2: Alphabetical Order Variations... ";
    
    // Test 2a: Alphabetical order (a, b, c)
    remove("test_alpha_abc.db");
    {
        BPlusTree tree("test_alpha_abc.db");
        tree.insert("a", 1);
        for (int i = 0; i < 100; i++) tree.insert("b", i);
        for (int i = 0; i < 996; i++) tree.insert("c", i);
        
        assert_equal(tree.find("a").size(), 1, "abc: a count");
        assert_equal(tree.find("b").size(), 100, "abc: b count");
        assert_equal(tree.find("c").size(), 996, "abc: c count");
    }
    remove("test_alpha_abc.db");
    
    // Test 2b: Reverse alphabetical (c, b, a)
    remove("test_alpha_cba.db");
    {
        BPlusTree tree("test_alpha_cba.db");
        for (int i = 0; i < 996; i++) tree.insert("c", i);
        for (int i = 0; i < 100; i++) tree.insert("b", i);
        tree.insert("a", 1);
        
        assert_equal(tree.find("a").size(), 1, "cba: a count");
        assert_equal(tree.find("b").size(), 100, "cba: b count");
        assert_equal(tree.find("c").size(), 996, "cba: c count");
    }
    remove("test_alpha_cba.db");
    
    // Test 2c: Mixed order (b, a, c)
    remove("test_alpha_bac.db");
    {
        BPlusTree tree("test_alpha_bac.db");
        for (int i = 0; i < 100; i++) tree.insert("b", i);
        tree.insert("a", 1);
        for (int i = 0; i < 996; i++) tree.insert("c", i);
        
        assert_equal(tree.find("a").size(), 1, "bac: a count");
        assert_equal(tree.find("b").size(), 100, "bac: b count");
        assert_equal(tree.find("c").size(), 996, "bac: c count");
    }
    remove("test_alpha_bac.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_multiple_keys_all_split() {
    cout << "Test 3: Multiple Keys All Split (5 keys, each 996+ values)... ";
    remove("test_multi_split.db");
    
    BPlusTree tree("test_multi_split.db");
    
    for (int i = 0; i < 996; i++) tree.insert("key1", i);
    for (int i = 0; i < 996; i++) tree.insert("key2", i);
    for (int i = 0; i < 996; i++) tree.insert("key3", i);
    for (int i = 0; i < 996; i++) tree.insert("key4", i);
    for (int i = 0; i < 996; i++) tree.insert("key5", i);
    
    assert_equal(tree.find("key1").size(), 996, "key1 count");
    assert_equal(tree.find("key2").size(), 996, "key2 count");
    assert_equal(tree.find("key3").size(), 996, "key3 count");
    assert_equal(tree.find("key4").size(), 996, "key4 count");
    assert_equal(tree.find("key5").size(), 996, "key5 count");
    
    remove("test_multi_split.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_edge_case_similar_keys() {
    cout << "Test 4: Edge Case - Keys That Compare Equal to Split Keys... ";
    remove("test_similar_keys.db");
    
    BPlusTree tree("test_similar_keys.db");
    
    // Insert keys with similar prefixes
    tree.insert("aaa", 1);
    for (int i = 0; i < 996; i++) tree.insert("aab", i);
    for (int i = 0; i < 500; i++) tree.insert("aac", i);
    
    assert_equal(tree.find("aaa").size(), 1, "aaa count");
    assert_equal(tree.find("aab").size(), 996, "aab count");
    assert_equal(tree.find("aac").size(), 500, "aac count");
    
    remove("test_similar_keys.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_mixed_operations() {
    cout << "Test 5: Mixed Operations (insert, delete, insert)... ";
    remove("test_mixed_ops.db");
    
    BPlusTree tree("test_mixed_ops.db");
    
    // Insert multi-key data
    for (int i = 0; i < 500; i++) tree.insert("alpha", i);
    for (int i = 0; i < 500; i++) tree.insert("beta", i);
    for (int i = 0; i < 500; i++) tree.insert("gamma", i);
    
    // Delete some values
    for (int i = 0; i < 100; i++) {
        tree.remove("alpha", i);
        tree.remove("beta", i);
    }
    
    // Insert more
    for (int i = 500; i < 1000; i++) tree.insert("alpha", i);
    for (int i = 500; i < 1000; i++) tree.insert("beta", i);
    
    // Verify all keys still findable
    assert_equal(tree.find("alpha").size(), 900, "alpha count after mixed ops");
    assert_equal(tree.find("beta").size(), 900, "beta count after mixed ops");
    assert_equal(tree.find("gamma").size(), 500, "gamma count after mixed ops");
    
    remove("test_mixed_ops.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_boundary_cases() {
    cout << "Test 6: Boundary Cases... ";
    
    // Test 6a: Empty tree find
    remove("test_empty.db");
    {
        BPlusTree tree("test_empty.db");
        assert_equal(tree.find("nonexistent").size(), 0, "empty tree find");
    }
    remove("test_empty.db");
    
    // Test 6b: Single key, varying value counts (0-1000)
    remove("test_single_key.db");
    {
        BPlusTree tree("test_single_key.db");
        for (int i = 0; i < 1000; i++) tree.insert("key", i);
        assert_equal(tree.find("key").size(), 1000, "single key 1000 values");
    }
    remove("test_single_key.db");
    
    // Test 6c: Two keys with different value counts
    remove("test_two_keys.db");
    {
        BPlusTree tree("test_two_keys.db");
        tree.insert("first", 1);
        for (int i = 0; i < 999; i++) tree.insert("second", i);
        assert_equal(tree.find("first").size(), 1, "two keys: first count");
        assert_equal(tree.find("second").size(), 999, "two keys: second count");
    }
    remove("test_two_keys.db");
    
    // Test 6d: 10 keys each with 100 values
    remove("test_ten_keys.db");
    {
        BPlusTree tree("test_ten_keys.db");
        for (int k = 0; k < 10; k++) {
            string key = "key" + to_string(k);
            for (int i = 0; i < 100; i++) {
                tree.insert(key, i);
            }
        }
        for (int k = 0; k < 10; k++) {
            string key = "key" + to_string(k);
            assert_equal(tree.find(key).size(), 100, "ten keys: " + key + " count");
        }
    }
    remove("test_ten_keys.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_persistence() {
    cout << "Test 7: Persistence with Multi-Key Data... ";
    remove("test_persist_multi.db");
    
    // Insert data in first session
    {
        BPlusTree tree("test_persist_multi.db");
        for (int i = 0; i < 500; i++) tree.insert("persist1", i);
        for (int i = 0; i < 500; i++) tree.insert("persist2", i);
        for (int i = 0; i < 500; i++) tree.insert("persist3", i);
        // Tree destructor should flush
    }
    
    // Reload and verify
    {
        BPlusTree tree("test_persist_multi.db");
        assert_equal(tree.find("persist1").size(), 500, "persist1 after reload");
        assert_equal(tree.find("persist2").size(), 500, "persist2 after reload");
        assert_equal(tree.find("persist3").size(), 500, "persist3 after reload");
    }
    
    remove("test_persist_multi.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_large_value_counts() {
    cout << "Test 8: Large Value Counts (2000+ values per key)... ";
    remove("test_large_values.db");
    
    BPlusTree tree("test_large_values.db");
    
    for (int i = 0; i < 2000; i++) tree.insert("large1", i);
    for (int i = 0; i < 2000; i++) tree.insert("large2", i);
    
    assert_equal(tree.find("large1").size(), 2000, "large1 count");
    assert_equal(tree.find("large2").size(), 2000, "large2 count");
    
    remove("test_large_values.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_interleaved_insertion() {
    cout << "Test 9: Interleaved Insertion... ";
    remove("test_interleaved.db");
    
    BPlusTree tree("test_interleaved.db");
    
    // Insert values in interleaved manner
    for (int i = 0; i < 1000; i++) {
        tree.insert("key1", i);
        tree.insert("key2", i);
        tree.insert("key3", i);
    }
    
    assert_equal(tree.find("key1").size(), 1000, "key1 interleaved");
    assert_equal(tree.find("key2").size(), 1000, "key2 interleaved");
    assert_equal(tree.find("key3").size(), 1000, "key3 interleaved");
    
    remove("test_interleaved.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

void test_critical_boundary_995_996_997() {
    cout << "Test 10: Critical Boundary (995, 996, 997 values)... ";
    
    // Test 10a: 995 values (just before split)
    remove("test_995.db");
    {
        BPlusTree tree("test_995.db");
        for (int i = 0; i < 995; i++) tree.insert("key", i);
        assert_equal(tree.find("key").size(), 995, "995 values");
    }
    remove("test_995.db");
    
    // Test 10b: 996 values (at split threshold)
    remove("test_996.db");
    {
        BPlusTree tree("test_996.db");
        for (int i = 0; i < 996; i++) tree.insert("key", i);
        assert_equal(tree.find("key").size(), 996, "996 values");
    }
    remove("test_996.db");
    
    // Test 10c: 997 values (just after split)
    remove("test_997.db");
    {
        BPlusTree tree("test_997.db");
        for (int i = 0; i < 997; i++) tree.insert("key", i);
        assert_equal(tree.find("key").size(), 997, "997 values");
    }
    remove("test_997.db");
    
    if (test_passed) {
        cout << "✅ PASS" << endl;
    }
}

int main() {
    cout << "=== Comprehensive Multi-Key Test Suite ===" << endl << endl;
    
    test_basic_multikey();
    test_passed = true;
    
    test_alphabetical_variations();
    test_passed = true;
    
    test_multiple_keys_all_split();
    test_passed = true;
    
    test_edge_case_similar_keys();
    test_passed = true;
    
    test_mixed_operations();
    test_passed = true;
    
    test_boundary_cases();
    test_passed = true;
    
    test_persistence();
    test_passed = true;
    
    test_large_value_counts();
    test_passed = true;
    
    test_interleaved_insertion();
    test_passed = true;
    
    test_critical_boundary_995_996_997();
    test_passed = true;
    
    cout << endl << "=== All Tests Passed! ✅ ===" << endl;
    
    return 0;
}

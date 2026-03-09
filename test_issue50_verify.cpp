#include "bplustree.h"
#include <iostream>
#include <cassert>
#include <cstdio>

using namespace std;

// Test specifically for issue #50: verify 3+ keys with 996+ values each
int main() {
    cout << "=== Issue #50 Verification Test ===" << endl;
    cout << "Testing 3+ keys with 996+ values after prev_leaf fix" << endl << endl;
    
    // Clean up any existing test database
    remove("test_issue50.db");
    
    // Test 1: 3 keys with 996 values each (minimum requirement)
    cout << "Test 1: Three keys with 996 values each... ";
    {
        BPlusTree tree("test_issue50.db");
        
        for (int i = 0; i < 996; i++) tree.insert("key1", i);
        for (int i = 0; i < 996; i++) tree.insert("key2", i);
        for (int i = 0; i < 996; i++) tree.insert("key3", i);
        
        auto result1 = tree.find("key1");
        auto result2 = tree.find("key2");
        auto result3 = tree.find("key3");
        
        if (result1.size() == 996 && result2.size() == 996 && result3.size() == 996) {
            cout << "✅ PASS" << endl;
            cout << "  key1: " << result1.size() << " values" << endl;
            cout << "  key2: " << result2.size() << " values" << endl;
            cout << "  key3: " << result3.size() << " values" << endl;
        } else {
            cout << "❌ FAIL" << endl;
            cout << "  key1: Expected 996, got " << result1.size() << endl;
            cout << "  key2: Expected 996, got " << result2.size() << endl;
            cout << "  key3: Expected 996, got " << result3.size() << endl;
            remove("test_issue50.db");
            return 1;
        }
    }
    remove("test_issue50.db");
    
    // Test 2: 5 keys with 996 values each (extended test)
    cout << endl << "Test 2: Five keys with 996 values each... ";
    {
        BPlusTree tree("test_issue50.db");
        
        for (int i = 0; i < 996; i++) tree.insert("key1", i);
        for (int i = 0; i < 996; i++) tree.insert("key2", i);
        for (int i = 0; i < 996; i++) tree.insert("key3", i);
        for (int i = 0; i < 996; i++) tree.insert("key4", i);
        for (int i = 0; i < 996; i++) tree.insert("key5", i);
        
        auto result1 = tree.find("key1");
        auto result2 = tree.find("key2");
        auto result3 = tree.find("key3");
        auto result4 = tree.find("key4");
        auto result5 = tree.find("key5");
        
        bool all_pass = true;
        if (result1.size() != 996) {
            cout << "❌ FAIL: key1: Expected 996, got " << result1.size() << endl;
            all_pass = false;
        }
        if (result2.size() != 996) {
            cout << "❌ FAIL: key2: Expected 996, got " << result2.size() << endl;
            all_pass = false;
        }
        if (result3.size() != 996) {
            cout << "❌ FAIL: key3: Expected 996, got " << result3.size() << endl;
            all_pass = false;
        }
        if (result4.size() != 996) {
            cout << "❌ FAIL: key4: Expected 996, got " << result4.size() << endl;
            all_pass = false;
        }
        if (result5.size() != 996) {
            cout << "❌ FAIL: key5: Expected 996, got " << result5.size() << endl;
            all_pass = false;
        }
        
        if (all_pass) {
            cout << "✅ PASS" << endl;
            cout << "  All 5 keys have exactly 996 values" << endl;
        } else {
            remove("test_issue50.db");
            return 1;
        }
    }
    remove("test_issue50.db");
    
    // Test 3: Tara's original case (small, medium, large)
    cout << endl << "Test 3: Tara's original multi-key case... ";
    {
        BPlusTree tree("test_issue50.db");
        
        tree.insert("small", 1);
        for (int i = 0; i < 100; i++) tree.insert("medium", i);
        for (int i = 0; i < 996; i++) tree.insert("large", i);
        
        auto result_small = tree.find("small");
        auto result_medium = tree.find("medium");
        auto result_large = tree.find("large");
        
        if (result_small.size() == 1 && result_medium.size() == 100 && result_large.size() == 996) {
            cout << "✅ PASS" << endl;
            cout << "  small: " << result_small.size() << " value" << endl;
            cout << "  medium: " << result_medium.size() << " values" << endl;
            cout << "  large: " << result_large.size() << " values" << endl;
        } else {
            cout << "❌ FAIL" << endl;
            cout << "  small: Expected 1, got " << result_small.size() << endl;
            cout << "  medium: Expected 100, got " << result_medium.size() << endl;
            cout << "  large: Expected 996, got " << result_large.size() << endl;
            remove("test_issue50.db");
            return 1;
        }
    }
    remove("test_issue50.db");
    
    // Test 4: 10 keys with 1000 values each (stress test)
    cout << endl << "Test 4: Ten keys with 1000 values each... ";
    {
        BPlusTree tree("test_issue50.db");
        
        for (int k = 0; k < 10; k++) {
            string key = "key" + to_string(k);
            for (int i = 0; i < 1000; i++) {
                tree.insert(key, i);
            }
        }
        
        bool all_pass = true;
        for (int k = 0; k < 10; k++) {
            string key = "key" + to_string(k);
            auto result = tree.find(key);
            if (result.size() != 1000) {
                cout << "❌ FAIL: " << key << ": Expected 1000, got " << result.size() << endl;
                all_pass = false;
            }
        }
        
        if (all_pass) {
            cout << "✅ PASS" << endl;
            cout << "  All 10 keys have exactly 1000 values" << endl;
        } else {
            remove("test_issue50.db");
            return 1;
        }
    }
    remove("test_issue50.db");
    
    cout << endl << "=== All Issue #50 Verification Tests Passed! ✅ ===" << endl;
    cout << "The prev_leaf fix correctly handles multi-key scenarios with 996+ values." << endl;
    
    return 0;
}

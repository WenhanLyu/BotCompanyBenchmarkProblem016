#include "bplustree.h"
#include <iostream>
#include <cstdio>

using namespace std;

int main() {
    remove("verify.db");
    
    cout << "Testing commit bcf348d fix..." << endl;
    cout << endl;
    
    // Test 1: Single key with 996 values (original bug from Lucas)
    cout << "Test 1: Single key with 996 values (Lucas's case)" << endl;
    {
        BPlusTree tree("verify.db");
        for (int i = 0; i < 996; i++) tree.insert("testkey", i);
        vector<int> result = tree.find("testkey");
        cout << "  Expected: 996 values" << endl;
        cout << "  Got: " << result.size() << " values" << endl;
        cout << "  Result: " << (result.size() == 996 ? "✅ PASS" : "❌ FAIL") << endl;
        remove("verify.db");
    }
    cout << endl;
    
    // Test 2: Three keys in alphabetical order (Tara's case - known to fail)
    cout << "Test 2: Three keys in alphabetical order (aaa, bbb, ccc)" << endl;
    {
        BPlusTree tree("verify.db");
        cout << "  Inserting 996 values for 'aaa'..." << endl;
        for (int i = 0; i < 996; i++) tree.insert("aaa", i);
        vector<int> r1 = tree.find("aaa");
        cout << "    aaa: " << r1.size() << " values" << endl;
        
        cout << "  Inserting 996 values for 'bbb'..." << endl;
        for (int i = 0; i < 996; i++) tree.insert("bbb", i);
        vector<int> r2a = tree.find("bbb");
        cout << "    bbb: " << r2a.size() << " values (before inserting ccc)" << endl;
        
        cout << "  Inserting 996 values for 'ccc'..." << endl;
        for (int i = 0; i < 996; i++) tree.insert("ccc", i);
        
        vector<int> r1_final = tree.find("aaa");
        vector<int> r2_final = tree.find("bbb");
        vector<int> r3_final = tree.find("ccc");
        
        cout << endl;
        cout << "  Final results:" << endl;
        cout << "    aaa: " << r1_final.size() << " values " 
             << (r1_final.size() == 996 ? "✅" : "❌") << endl;
        cout << "    bbb: " << r2_final.size() << " values " 
             << (r2_final.size() == 996 ? "✅" : "❌") << endl;
        cout << "    ccc: " << r3_final.size() << " values " 
             << (r3_final.size() == 996 ? "✅" : "❌") << endl;
        
        if (r1_final.size() == 996 && r2_final.size() == 996 && r3_final.size() == 996) {
            cout << endl << "  Result: ✅ PASS - All keys found" << endl;
        } else {
            cout << endl << "  Result: ❌ FAIL - Middle key lost " 
                 << (996 - r2_final.size()) << " values" << endl;
        }
        remove("verify.db");
    }
    cout << endl;
    
    // Test 3: Three keys in reverse order (should work)
    cout << "Test 3: Three keys in reverse order (ccc, bbb, aaa)" << endl;
    {
        BPlusTree tree("verify.db");
        for (int i = 0; i < 996; i++) tree.insert("ccc", i);
        for (int i = 0; i < 996; i++) tree.insert("bbb", i);
        for (int i = 0; i < 996; i++) tree.insert("aaa", i);
        
        vector<int> r1 = tree.find("aaa");
        vector<int> r2 = tree.find("bbb");
        vector<int> r3 = tree.find("ccc");
        
        cout << "  aaa: " << r1.size() << " values" << endl;
        cout << "  bbb: " << r2.size() << " values" << endl;
        cout << "  ccc: " << r3.size() << " values" << endl;
        cout << "  Result: " << (r1.size() == 996 && r2.size() == 996 && r3.size() == 996 
                                  ? "✅ PASS" : "❌ FAIL") << endl;
        remove("verify.db");
    }
    cout << endl;
    
    cout << "=== Summary ===" << endl;
    cout << "Commit bcf348d fixes the single-key 996-value bug (Lucas's case)" << endl;
    cout << "But does NOT fix the multi-key alphabetical order bug (Tara's case)" << endl;
    
    return 0;
}

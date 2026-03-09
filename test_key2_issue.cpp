#include <iostream>
#include <vector>
#include <set>
#include "bplustree.h"

int main() {
    remove("test_key2.db");
    
    BPlusTree tree("test_key2.db");
    
    // Insert the pattern from key2_values.txt
    // Insert 996 values each for key1, key2, key3
    
    std::cout << "Inserting 996 values for key1..." << std::endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("key1", i);
    }
    
    std::cout << "Inserting 996 values for key2..." << std::endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("key2", i);
    }
    
    std::cout << "Inserting 996 values for key3..." << std::endl;
    for (int i = 0; i < 996; i++) {
        tree.insert("key3", i);
    }
    
    std::cout << "\nFinding key2..." << std::endl;
    std::vector<int> result = tree.find("key2");
    
    std::cout << "Found " << result.size() << " values for key2" << std::endl;
    
    if (result.size() != 996) {
        std::cout << "❌ ERROR: Expected 996 values, got " << result.size() << std::endl;
        
        // Check which values are present
        std::set<int> found(result.begin(), result.end());
        std::vector<int> missing;
        for (int i = 0; i < 996; i++) {
            if (found.find(i) == found.end()) {
                missing.push_back(i);
            }
        }
        
        std::cout << "\nMissing " << missing.size() << " values" << std::endl;
        if (!missing.empty()) {
            std::cout << "Missing value ranges: ";
            int range_start = missing[0];
            int range_end = missing[0];
            for (size_t i = 1; i < missing.size(); i++) {
                if (missing[i] == range_end + 1) {
                    range_end = missing[i];
                } else {
                    if (range_start == range_end) {
                        std::cout << range_start << " ";
                    } else {
                        std::cout << range_start << "-" << range_end << " ";
                    }
                    range_start = missing[i];
                    range_end = missing[i];
                }
            }
            if (range_start == range_end) {
                std::cout << range_start;
            } else {
                std::cout << range_start << "-" << range_end;
            }
            std::cout << std::endl;
        }
        
        // Show what we found
        std::cout << "\nValues found: ";
        for (int i = 0; i < std::min(50, (int)result.size()); i++) {
            std::cout << result[i] << " ";
        }
        if (result.size() > 50) {
            std::cout << "... (showing first 50)";
        }
        std::cout << std::endl;
    } else {
        std::cout << "✅ PASS: Found all 996 values for key2" << std::endl;
    }
    
    remove("test_key2.db");
    return 0;
}

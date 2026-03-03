// B+ Tree File Storage Database
// ACMOJ Problem 2697

#include "bplustree.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
    try {
        // Create or open database file
        BPlusTree tree("database.db");
        
        int n;
        std::cin >> n;
        
        for (int i = 0; i < n; i++) {
            std::string cmd;
            std::cin >> cmd;
            
            if (cmd == "insert") {
                std::string key;
                int value;
                std::cin >> key >> value;
                tree.insert(key, value);
            }
            else if (cmd == "find") {
                std::string key;
                std::cin >> key;
                std::vector<int> results = tree.find(key);
                
                if (results.empty()) {
                    std::cout << "null" << std::endl;
                } else {
                    for (size_t j = 0; j < results.size(); j++) {
                        if (j > 0) std::cout << " ";
                        std::cout << results[j];
                    }
                    std::cout << std::endl;
                }
            }
            else if (cmd == "delete") {
                std::string key;
                int value;
                std::cin >> key >> value;
                tree.remove(key, value);
            }
        }
        
        // Flush changes to disk before exiting
        tree.flush();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

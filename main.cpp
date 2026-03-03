// B+ Tree File Storage Database
// ACMOJ Problem 2697
//
// This is a placeholder main.cpp for build system testing.
// The actual implementation will be added in subsequent cycles.

#include <iostream>
#include <string>
#include <vector>

int main() {
    // Placeholder implementation
    // TODO: Implement B+ tree operations
    
    int n;
    std::cin >> n;
    
    for (int i = 0; i < n; i++) {
        std::string cmd;
        std::cin >> cmd;
        
        if (cmd == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            // TODO: tree.insert(key, value);
        }
        else if (cmd == "find") {
            std::string key;
            std::cin >> key;
            // TODO: auto results = tree.find(key);
            std::cout << "null" << std::endl;
        }
        else if (cmd == "delete") {
            std::string key;
            int value;
            std::cin >> key >> value;
            // TODO: tree.delete(key, value);
        }
    }
    
    return 0;
}

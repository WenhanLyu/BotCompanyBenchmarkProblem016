#include "bplustree.h"
#include <iostream>
#include <string>

int main() {
    BPlusTree tree("ultra3.db");
    
    int n;
    std::cin >> n;
    std::cerr << "n = " << n << std::endl;
    
    for (int i = 0; i < n; i++) {
        std::string cmd;
        std::cin >> cmd;
        std::cerr << "cmd = " << cmd << std::endl;
        
        if (cmd == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            std::cerr << "Inserting " << key << " " << value << std::endl;
            tree.insert(key, value);
            std::cerr << "Insert done" << std::endl;
        }
    }
    
    std::cerr << "Flushing..." << std::endl;
    tree.flush();
    std::cerr << "Done" << std::endl;
    
    return 0;
}

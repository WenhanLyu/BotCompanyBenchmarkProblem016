#include "file_manager.h"
#include "bplustree.h"
#include <iostream>

int main() {
    std::cerr << "Starting..." << std::endl;
    {
        BPlusTree tree("ultra.db");
        std::cerr << "Created tree" << std::endl;
    }
    std::cerr << "Destroyed tree" << std::endl;
    return 0;
}

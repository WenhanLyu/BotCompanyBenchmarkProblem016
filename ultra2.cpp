#include "bplustree.h"
#include <iostream>

int main() {
    std::cerr << "Creating tree..." << std::endl;
    {
        BPlusTree tree("ultra2.db");
        std::cerr << "Inserting..." << std::endl;
        tree.insert("test", 123);
        std::cerr << "Insert done" << std::endl;
    }
    std::cerr << "Tree destroyed" << std::endl;
    return 0;
}

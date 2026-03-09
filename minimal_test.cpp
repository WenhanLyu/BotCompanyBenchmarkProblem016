#include "bplustree.h"
#include <iostream>

int main() {
    std::cerr << "Starting test..." << std::endl;
    BPlusTree tree("test.db");
    std::cerr << "Tree created" << std::endl;
    tree.insert("test", 123);
    std::cerr << "Insert done" << std::endl;
    return 0;
}

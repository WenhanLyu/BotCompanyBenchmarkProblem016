#include "bplustree.h"
#include <iostream>

int main() {
    std::cerr << "[1] Creating tree..." << std::endl;
    BPlusTree tree("test_debug.db");
    std::cerr << "[2] Inserting..." << std::endl;
    tree.insert("test", 123);
    std::cerr << "[3] Finding..." << std::endl;
    auto results = tree.find("test");
    std::cerr << "[4] Results: " << results.size() << std::endl;
    if (!results.empty()) {
        std::cerr << "[5] Value: " << results[0] << std::endl;
    }
    std::cerr << "[6] Done" << std::endl;
    return 0;
}

#include "bplustree.h"
#include <iostream>
#include <fstream>

int main() {
    // Test that tree can be safely deleted in exception path
    BPlusTree* tree = nullptr;
    
    try {
        tree = new BPlusTree("test_exception.db");
        
        // Insert some data
        tree->insert("key1", 100);
        tree->insert("key2", 200);
        
        // Simulate an exception
        throw std::runtime_error("Test exception to verify cleanup");
        
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
        
        // This should NOT crash - destructor is noexcept
        delete tree;
        tree = nullptr;
        
        std::cout << "Tree deleted successfully in exception handler" << std::endl;
    }
    
    // Clean up test file
    std::remove("test_exception.db");
    
    std::cout << "Test PASSED - no crashes" << std::endl;
    return 0;
}

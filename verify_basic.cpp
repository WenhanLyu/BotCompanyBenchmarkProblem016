#include "file_manager.h"
#include "node.h"
#include <iostream>

int main() {
    try {
        std::cout << "Creating FileManager..." << std::endl;
        FileManager fm("test_verify.db");
        fm.open();
        std::cout << "FileManager opened" << std::endl;
        
        // Create a simple leaf node
        LeafNode* leaf = new LeafNode();
        leaf->page_id = -1;  // Not yet assigned
        leaf->keys.push_back("test_key");
        leaf->values.push_back(42);
        
        std::cout << "Writing node..." << std::endl;
        fm.writeNode(leaf);
        std::cout << "Node written with page_id: " << leaf->page_id << std::endl;
        
        fm.flush();
        std::cout << "Flushed to disk" << std::endl;
        
        // Read back
        int page_id = leaf->page_id;
        Node* read_node = fm.readNode(page_id);
        LeafNode* read_leaf = dynamic_cast<LeafNode*>(read_node);
        
        std::cout << "Read node back - key: " << read_leaf->keys[0] 
                  << ", value: " << read_leaf->values[0] << std::endl;
        
        std::cout << "Test completed successfully!" << std::endl;
        std::cout << "Destructor will now be called..." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "After try block - destructor succeeded without throwing" << std::endl;
    return 0;
}

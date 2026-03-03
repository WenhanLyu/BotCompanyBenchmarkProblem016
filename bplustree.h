#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "node.h"
#include "file_manager.h"
#include <string>
#include <vector>

// BPlusTree: Main B+ tree implementation with file-based persistence
class BPlusTree {
private:
    FileManager* file_manager;
    bool owns_file_manager;  // Track if we need to delete file_manager
    
    // Helper: Find the leaf node that should contain the given key
    // Returns the page_id of the leaf node
    int findLeaf(const std::string& key);
    
    // Helper: Insert into a leaf node and handle splits if necessary
    // Returns the split key and new right page_id if split occurred
    // Otherwise returns empty string and -1
    struct SplitResult {
        std::string split_key;
        int right_page_id;
        
        SplitResult() : split_key(""), right_page_id(-1) {}
        SplitResult(const std::string& key, int page_id) 
            : split_key(key), right_page_id(page_id) {}
        
        bool didSplit() const { return right_page_id != -1; }
    };
    
    // Recursive insert helper
    SplitResult insertInternal(int page_id, const std::string& key, int value);
    
    // Split an internal node
    SplitResult splitInternalNode(InternalNode* node);
    
    // Split a leaf node
    SplitResult splitLeafNode(LeafNode* node);
    
public:
    // Constructor: opens or creates a database file
    BPlusTree(const std::string& filename);
    
    // Constructor: use existing FileManager (for testing)
    BPlusTree(FileManager* fm);
    
    // Destructor
    ~BPlusTree();
    
    // Insert a key-value pair
    // If the key already exists, adds the value to the existing entry
    // Does not allow duplicate values for the same key
    void insert(const std::string& key, int value);
    
    // Find all values for a given key
    // Returns a sorted vector of values (empty if key not found)
    std::vector<int> find(const std::string& key);
    
    // Remove a specific value from a key
    // Returns true if the value was found and removed, false otherwise
    bool remove(const std::string& key, int value);
    
    // Flush all changes to disk
    void flush();
    
    // Print cache statistics (for debugging/testing)
    void printCacheStats() const { file_manager->printCacheStats(); }
};

#endif // BPLUSTREE_H

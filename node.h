#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

// Configuration constants
constexpr int PAGE_SIZE = 4096;  // 4KB page size
constexpr int MAX_KEY_SIZE = 64;  // Maximum key size in bytes

// Forward declarations
class InternalNode;
class LeafNode;

// LeafEntry: stores a key and its associated values (sorted vector of ints)
struct LeafEntry {
    char key[MAX_KEY_SIZE + 1];  // +1 for null terminator
    std::vector<int> values;      // Sorted vector of values
    
    LeafEntry();
    LeafEntry(const std::string& k, int value);
    
    // Add a value (maintains sorted order, no duplicates)
    void addValue(int value);
    
    // Remove a value
    bool removeValue(int value);
    
    // Check if a value exists
    bool hasValue(int value) const;
    
    // Get the key as a string
    std::string getKey() const;
};

// Base class for nodes
class Node {
public:
    bool is_leaf;
    int page_id;  // Position in file (page number)
    
    Node(bool leaf) : is_leaf(leaf), page_id(-1) {}
    virtual ~Node() {}
    
    virtual int serialize(char* buffer) const = 0;
    virtual int deserialize(const char* buffer) = 0;
    virtual Node* clone() const = 0;  // Create a deep copy of the node
};

// InternalNode: stores keys and child page IDs
class InternalNode : public Node {
public:
    std::vector<std::string> keys;
    std::vector<int> children;  // Page IDs of children
    
    InternalNode();
    
    // Binary search to find the correct child index for a key
    int findChildIndex(const std::string& key) const;
    
    // Insert a key and right child (used during node splits)
    void insertKey(const std::string& key, int right_child);
    
    // Serialize to buffer, returns bytes written
    int serialize(char* buffer) const override;
    
    // Deserialize from buffer, returns bytes read
    int deserialize(const char* buffer) override;
    
    // Check if node needs to be split
    bool needsSplit() const;
    
    // Create a deep copy of this node
    Node* clone() const override;
};

// LeafNode: stores actual data entries
class LeafNode : public Node {
public:
    std::vector<LeafEntry> entries;
    int next_leaf;  // Page ID of next leaf (-1 if none)
    int prev_leaf;  // Page ID of previous leaf (-1 if none)
    
    LeafNode();
    
    // Find entry index by key (returns -1 if not found)
    int findEntry(const std::string& key) const;
    
    // Insert or update an entry
    void insertEntry(const std::string& key, int value);
    
    // Delete a value from an entry
    bool deleteEntry(const std::string& key, int value);
    
    // Get all values for a key
    std::vector<int> getValues(const std::string& key) const;
    
    // Serialize to buffer, returns bytes written
    int serialize(char* buffer) const override;
    
    // Deserialize from buffer, returns bytes read
    int deserialize(const char* buffer) override;
    
    // Check if node needs to be split
    bool needsSplit() const;
    
    // Split this leaf node, returns new right node
    LeafNode* split();
    
    // Create a deep copy of this node
    Node* clone() const override;
};

#endif // NODE_H

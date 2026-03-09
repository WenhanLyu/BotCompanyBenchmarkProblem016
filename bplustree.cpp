#include "bplustree.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

// ============================================================================
// Constructor & Destructor
// ============================================================================

BPlusTree::BPlusTree(const std::string& filename) {
    file_manager = new FileManager(filename);
    file_manager->open();
    owns_file_manager = true;
}

BPlusTree::BPlusTree(FileManager* fm) {
    file_manager = fm;
    owns_file_manager = false;
}

BPlusTree::~BPlusTree() {
    if (owns_file_manager) {
        delete file_manager;
    }
}

// ============================================================================
// Public Interface
// ============================================================================

void BPlusTree::insert(const std::string& key, int value) {
    // Validate key length
    if (key.length() > MAX_KEY_SIZE) {
        // Key is too long, truncate or reject
        // For safety, we'll truncate to MAX_KEY_SIZE
        std::string truncated_key = key.substr(0, MAX_KEY_SIZE);
        insert(truncated_key, value);
        return;
    }
    
    int root_page_id = file_manager->getRootPageId();
    
    if (root_page_id == -1) {
        // Empty tree - create a new leaf node as root
        LeafNode* root = new LeafNode();
        root->insertEntry(key, value);
        file_manager->writeNode(root);
        file_manager->setRootPageId(root->page_id);
        // Don't delete root - FileManager owns it now
        return;
    }
    
    // Insert into existing tree
    SplitResult result = insertInternal(root_page_id, key, value);
    
    // If root split, create a new root
    if (result.didSplit()) {
        InternalNode* new_root = new InternalNode();
        new_root->children.push_back(root_page_id);
        new_root->keys.push_back(result.split_key);
        new_root->children.push_back(result.right_page_id);
        
        file_manager->writeNode(new_root);
        file_manager->setRootPageId(new_root->page_id);
        
        // Don't delete new_root - FileManager owns it now
    }
}

std::vector<int> BPlusTree::find(const std::string& key) {
    int root_page_id = file_manager->getRootPageId();
    
    if (root_page_id == -1) {
        // Empty tree
        return std::vector<int>();
    }
    
    int leaf_page_id = findLeaf(key);
    
    if (leaf_page_id == -1) {
        return std::vector<int>();
    }
    
    // Collect values from the current leaf and ALL adjacent leaves containing this key
    // We need to scan both backward and forward because findLeaf() may land anywhere
    // in the sequence of leaves containing the key (due to value-level splits)
    std::vector<int> all_values;
    
    // First, collect from the starting leaf
    Node* start_node = file_manager->readNode(leaf_page_id);
    LeafNode* start_leaf = dynamic_cast<LeafNode*>(start_node);
    if (!start_leaf) {
        return all_values;  // Empty result
    }
    
    std::vector<int> start_values = start_leaf->getValues(key);
    if (start_values.empty()) {
        return all_values;  // Key not found
    }
    
    // Scan backward to collect from previous leaves
    // CRITICAL: Don't stop just because a leaf has 0 values for our key!
    // Value-level splits can insert leaves with different keys in the middle of the chain.
    // We must scan the ENTIRE backward chain to find all occurrences.
    int prev_id = start_leaf->prev_leaf;
    while (prev_id != -1) {
        Node* node = file_manager->readNode(prev_id);
        LeafNode* leaf = dynamic_cast<LeafNode*>(node);
        if (!leaf) break;
        
        // Collect values if they exist
        std::vector<int> values = leaf->getValues(key);
        if (!values.empty()) {
            // Prepend values (since we're going backward)
            all_values.insert(all_values.begin(), values.begin(), values.end());
        }
        
        prev_id = leaf->prev_leaf;
    }
    
    // Add values from the starting leaf
    all_values.insert(all_values.end(), start_values.begin(), start_values.end());
    
    // Scan forward to collect from next leaves
    // CRITICAL: Don't stop just because a leaf has 0 values for our key!
    // Value-level splits can insert leaves with different keys in the middle of the chain.
    // We must scan the ENTIRE forward chain to find all occurrences.
    int next_id = start_leaf->next_leaf;
    while (next_id != -1) {
        Node* node = file_manager->readNode(next_id);
        LeafNode* leaf = dynamic_cast<LeafNode*>(node);
        if (!leaf) break;
        
        // Collect values if they exist
        std::vector<int> values = leaf->getValues(key);
        if (!values.empty()) {
            all_values.insert(all_values.end(), values.begin(), values.end());
        }
        
        next_id = leaf->next_leaf;
    }
    
    // Sort the final result to ensure ascending order
    std::sort(all_values.begin(), all_values.end());
    
    return all_values;
}

bool BPlusTree::remove(const std::string& key, int value) {
    int root_page_id = file_manager->getRootPageId();
    
    if (root_page_id == -1) {
        // Empty tree
        return false;
    }
    
    int leaf_page_id = findLeaf(key);
    
    if (leaf_page_id == -1) {
        return false;
    }
    
    // Traverse multiple leaves to find the specific key-value pair
    // Use the same pattern as find()
    while (leaf_page_id != -1) {
        // Read the leaf node
        Node* node = file_manager->readNode(leaf_page_id);
        LeafNode* leaf = dynamic_cast<LeafNode*>(node);
        
        if (!leaf) {
            // Don't delete node - FileManager owns it
            break;
        }
        
        // Check if this leaf contains any entries with the key
        std::vector<int> values = leaf->getValues(key);
        
        // If no values found in this leaf, we've gone past the key
        if (values.empty()) {
            // Don't delete leaf - FileManager owns it
            break;
        }
        
        // Try to delete the value from this leaf
        bool removed = leaf->deleteEntry(key, value);
        
        if (removed) {
            // Write the updated leaf back to disk
            file_manager->writeNode(leaf);
            // Don't delete leaf - FileManager owns it
            return true;
        }
        
        // Move to next leaf to continue searching
        leaf_page_id = leaf->next_leaf;
        // Don't delete leaf - FileManager owns it
    }
    
    return false;
}

void BPlusTree::flush() {
    file_manager->flush();
}

// ============================================================================
// Private Helper Methods
// ============================================================================

int BPlusTree::findLeaf(const std::string& key) {
    int current_page_id = file_manager->getRootPageId();
    
    if (current_page_id == -1) {
        return -1;
    }
    
    while (true) {
        Node* node = file_manager->readNode(current_page_id);
        
        if (!node) {
            // Failed to read node
            return -1;
        }
        
        if (node->is_leaf) {
            int leaf_page_id = node->page_id;
            // Don't delete node - FileManager owns it
            return leaf_page_id;
        }
        
        // Internal node - find the appropriate child
        InternalNode* internal = dynamic_cast<InternalNode*>(node);
        if (!internal) {
            // Don't delete node - FileManager owns it
            return -1;
        }
        
        int child_index = internal->findChildIndex(key);
        
        // Bounds check on children array access
        if (child_index < 0 || child_index >= (int)internal->children.size()) {
            // Invalid child index, corrupted node
            return -1;
        }
        
        current_page_id = internal->children[child_index];
        // Don't delete internal - FileManager owns it
    }
}

BPlusTree::SplitResult BPlusTree::insertInternal(int page_id, const std::string& key, int value) {
    Node* node = file_manager->readNode(page_id);
    
    if (!node) {
        // Failed to read node
        return SplitResult();
    }
    
    if (node->is_leaf) {
        // Leaf node - insert the value
        LeafNode* leaf = dynamic_cast<LeafNode*>(node);
        if (!leaf) {
            // Cast failed - corrupted node
            return SplitResult();
        }
        
        leaf->insertEntry(key, value);
        
        // Check if we need to split
        if (leaf->needsSplit()) {
            SplitResult result = splitLeafNode(leaf);
            file_manager->writeNode(leaf);  // Write updated left node
            // Don't delete leaf - FileManager owns it
            return result;
        } else {
            // No split needed, just write back
            file_manager->writeNode(leaf);
            // Don't delete leaf - FileManager owns it
            return SplitResult();
        }
    } else {
        // Internal node - find the appropriate child
        InternalNode* internal = dynamic_cast<InternalNode*>(node);
        if (!internal) {
            // Cast failed - corrupted node
            return SplitResult();
        }
        
        int child_index = internal->findChildIndex(key);
        
        // Bounds check on children array access
        if (child_index < 0 || child_index >= (int)internal->children.size()) {
            // Invalid child index, corrupted node
            return SplitResult();
        }
        
        int child_page_id = internal->children[child_index];
        
        // Recursively insert into child
        SplitResult child_result = insertInternal(child_page_id, key, value);
        
        if (child_result.didSplit()) {
            // Child split - insert the split key into this node
            internal->insertKey(child_result.split_key, child_result.right_page_id);
            
            // Check if this node needs to split
            if (internal->needsSplit()) {
                SplitResult result = splitInternalNode(internal);
                file_manager->writeNode(internal);  // Write updated left node
                // Don't delete internal - FileManager owns it
                return result;
            } else {
                // No split needed, just write back
                file_manager->writeNode(internal);
                // Don't delete internal - FileManager owns it
                return SplitResult();
            }
        } else {
            // No split in child, nothing to do
            // Don't delete internal - FileManager owns it
            return SplitResult();
        }
    }
}

BPlusTree::SplitResult BPlusTree::splitLeafNode(LeafNode* left_node) {
    if (!left_node) {
        // Invalid node pointer
        return SplitResult();
    }
    
    // Create new right node
    LeafNode* right_node = left_node->split();
    
    if (!right_node || right_node->entries.empty()) {
        // Split failed or produced invalid node
        return SplitResult();
    }
    
    std::string left_key = left_node->entries.empty() ? "?" : left_node->entries[0].getKey();
    std::string right_key = right_node->entries.empty() ? "?" : right_node->entries[0].getKey();
    std::cerr << "[SPLIT LEAF] " << left_key << "(" << left_node->page_id << ") -> " 
              << right_key << "(new) | old_next=" << left_node->next_leaf << std::endl;
    
    // Update leaf chain pointers (both next and prev)
    right_node->next_leaf = left_node->next_leaf;
    right_node->prev_leaf = left_node->page_id;
    file_manager->writeNode(right_node);  // Write right node to get its page_id (FileManager takes ownership)
    
    std::cerr << "[SPLIT LEAF] right_node created with page_id=" << right_node->page_id 
              << " next=" << right_node->next_leaf << " prev=" << right_node->prev_leaf << std::endl;
    
    // Update the next node's prev_leaf pointer if it exists
    if (right_node->next_leaf != -1) {
        LeafNode* next_node = dynamic_cast<LeafNode*>(file_manager->readNode(right_node->next_leaf));
        if (next_node) {
            std::cerr << "[SPLIT LEAF] Updating next node " << right_node->next_leaf 
                      << " prev_leaf to " << right_node->page_id << std::endl;
            next_node->prev_leaf = right_node->page_id;
            file_manager->writeNode(next_node);
        }
    }
    
    left_node->next_leaf = right_node->page_id;
    
    // The split key is the first key of the right node
    std::string split_key = right_node->entries[0].getKey();
    int right_page_id = right_node->page_id;
    
    // Don't delete right_node - FileManager owns it now
    
    return SplitResult(split_key, right_page_id);
}

// Helper: Find the rightmost leaf in a subtree rooted at page_id
int BPlusTree::findRightmostLeaf(int page_id) {
    if (page_id == -1) {
        return -1;
    }
    
    Node* node = file_manager->readNode(page_id);
    if (!node) {
        return -1;
    }
    
    if (node->is_leaf) {
        int leaf_page_id = node->page_id;
        return leaf_page_id;
    }
    
    // Internal node - traverse to rightmost child
    InternalNode* internal = static_cast<InternalNode*>(node);
    if (internal->children.empty()) {
        return -1;
    }
    
    int rightmost_child = internal->children.back();
    return findRightmostLeaf(rightmost_child);
}

// Helper: Find the leftmost leaf in a subtree rooted at page_id
int BPlusTree::findLeftmostLeaf(int page_id) {
    if (page_id == -1) {
        return -1;
    }
    
    Node* node = file_manager->readNode(page_id);
    if (!node) {
        return -1;
    }
    
    if (node->is_leaf) {
        int leaf_page_id = node->page_id;
        return leaf_page_id;
    }
    
    // Internal node - traverse to leftmost child
    InternalNode* internal = static_cast<InternalNode*>(node);
    if (internal->children.empty()) {
        return -1;
    }
    
    int leftmost_child = internal->children.front();
    return findLeftmostLeaf(leftmost_child);
}

BPlusTree::SplitResult BPlusTree::splitInternalNode(InternalNode* left_node) {
    std::cerr << "[SPLIT INTERNAL] called" << std::endl;
    if (!left_node || left_node->keys.empty()) {
        // Invalid node pointer or empty node
        return SplitResult();
    }
    
    // Split the internal node
    InternalNode* right_node = new InternalNode();
    
    int mid = left_node->keys.size() / 2;
    
    // Bounds check to ensure mid is valid
    if (mid < 0 || mid >= (int)left_node->keys.size()) {
        delete right_node;
        return SplitResult();
    }
    
    // The middle key moves up to the parent
    std::string split_key = left_node->keys[mid];
    
    // Ensure we have enough children for the split
    if ((int)left_node->children.size() < mid + 2) {
        delete right_node;
        return SplitResult();
    }
    
    // Move keys and children to the right node
    // Right node gets keys [mid+1, end) and children [mid+1, end]
    right_node->keys.assign(left_node->keys.begin() + mid + 1, left_node->keys.end());
    right_node->children.assign(left_node->children.begin() + mid + 1, left_node->children.end());
    
    // Left node keeps keys [0, mid) and children [0, mid+1)
    left_node->keys.erase(left_node->keys.begin() + mid, left_node->keys.end());
    left_node->children.erase(left_node->children.begin() + mid + 1, left_node->children.end());
    
    // Write right node to get its page_id (FileManager takes ownership)
    file_manager->writeNode(right_node);
    int right_page_id = right_node->page_id;
    
    // CRITICAL FIX: Maintain leaf chain after internal node split
    // Find the rightmost leaf in the left subtree and leftmost leaf in the right subtree
    // and link them together to prevent breaking the leaf chain
    int rightmost_left_page = findRightmostLeaf(left_node->children.back());
    int leftmost_right_page = findLeftmostLeaf(right_node->children.front());
    
    if (rightmost_left_page != -1 && leftmost_right_page != -1) {
        // Read both leaves
        Node* rightmost_node = file_manager->readNode(rightmost_left_page);
        Node* leftmost_node = file_manager->readNode(leftmost_right_page);
        
        if (rightmost_node && rightmost_node->is_leaf && 
            leftmost_node && leftmost_node->is_leaf) {
            LeafNode* rightmost_leaf = static_cast<LeafNode*>(rightmost_node);
            LeafNode* leftmost_leaf = static_cast<LeafNode*>(leftmost_node);
            
            std::cerr << "[SPLIT INTERNAL] Linking leaves: " << rightmost_leaf->page_id 
                      << " -> " << leftmost_leaf->page_id << std::endl;
            
            // Link them together
            rightmost_leaf->next_leaf = leftmost_leaf->page_id;
            leftmost_leaf->prev_leaf = rightmost_leaf->page_id;
            
            // Write both leaves back to persist the links
            file_manager->writeNode(rightmost_leaf);
            file_manager->writeNode(leftmost_leaf);
        }
    }
    
    // Don't delete right_node - FileManager owns it now
    
    return SplitResult(split_key, right_page_id);
}

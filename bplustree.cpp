#include "bplustree.h"
#include <stdexcept>
#include <algorithm>

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
    
    // Read the leaf node and get values
    Node* node = file_manager->readNode(leaf_page_id);
    LeafNode* leaf = dynamic_cast<LeafNode*>(node);
    
    if (!leaf) {
        // Don't delete node - FileManager owns it
        return std::vector<int>();
    }
    
    std::vector<int> values = leaf->getValues(key);
    // Don't delete leaf - FileManager owns it
    
    return values;
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
    
    // Read the leaf node
    Node* node = file_manager->readNode(leaf_page_id);
    LeafNode* leaf = dynamic_cast<LeafNode*>(node);
    
    if (!leaf) {
        // Don't delete node - FileManager owns it
        return false;
    }
    
    // Try to delete the value
    bool removed = leaf->deleteEntry(key, value);
    
    if (removed) {
        // Write the updated leaf back to disk
        file_manager->writeNode(leaf);
    }
    
    // Don't delete leaf - FileManager owns it
    return removed;
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
        current_page_id = internal->children[child_index];
        // Don't delete internal - FileManager owns it
    }
}

BPlusTree::SplitResult BPlusTree::insertInternal(int page_id, const std::string& key, int value) {
    Node* node = file_manager->readNode(page_id);
    
    if (node->is_leaf) {
        // Leaf node - insert the value
        LeafNode* leaf = dynamic_cast<LeafNode*>(node);
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
        int child_index = internal->findChildIndex(key);
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
    // Create new right node
    LeafNode* right_node = left_node->split();
    
    // Update leaf chain pointers
    right_node->next_leaf = left_node->next_leaf;
    file_manager->writeNode(right_node);  // Write right node to get its page_id (FileManager takes ownership)
    left_node->next_leaf = right_node->page_id;
    
    // The split key is the first key of the right node
    std::string split_key = right_node->entries[0].getKey();
    int right_page_id = right_node->page_id;
    
    // Don't delete right_node - FileManager owns it now
    
    return SplitResult(split_key, right_page_id);
}

BPlusTree::SplitResult BPlusTree::splitInternalNode(InternalNode* left_node) {
    // Split the internal node
    InternalNode* right_node = new InternalNode();
    
    int mid = left_node->keys.size() / 2;
    
    // The middle key moves up to the parent
    std::string split_key = left_node->keys[mid];
    
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
    
    // Don't delete right_node - FileManager owns it now
    
    return SplitResult(split_key, right_page_id);
}

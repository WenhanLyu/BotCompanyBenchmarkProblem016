#include "node.h"
#include <cstring>
#include <algorithm>

// ============================================================================
// LeafEntry Implementation
// ============================================================================

LeafEntry::LeafEntry() {
    memset(key, 0, MAX_KEY_SIZE + 1);
}

LeafEntry::LeafEntry(const std::string& k, int value) {
    memset(key, 0, MAX_KEY_SIZE + 1);
    strncpy(key, k.c_str(), MAX_KEY_SIZE);
    values.push_back(value);
}

void LeafEntry::addValue(int value) {
    // Binary search to find insertion point
    auto it = std::lower_bound(values.begin(), values.end(), value);
    // Only insert if not already present (no duplicates)
    if (it == values.end() || *it != value) {
        values.insert(it, value);
    }
}

bool LeafEntry::removeValue(int value) {
    auto it = std::lower_bound(values.begin(), values.end(), value);
    if (it != values.end() && *it == value) {
        values.erase(it);
        return true;
    }
    return false;
}

bool LeafEntry::hasValue(int value) const {
    return std::binary_search(values.begin(), values.end(), value);
}

std::string LeafEntry::getKey() const {
    return std::string(key);
}

// ============================================================================
// InternalNode Implementation
// ============================================================================

InternalNode::InternalNode() : Node(false) {}

int InternalNode::findChildIndex(const std::string& key) const {
    // Binary search to find the appropriate child
    // When key equals a split key, prefer left child to find first occurrence
    int left = 0, right = keys.size();
    while (left < right) {
        int mid = (left + right) / 2;
        if (key <= keys[mid]) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    return left;
}

void InternalNode::insertKey(const std::string& key, int right_child) {
    // Find insertion point
    auto it = std::lower_bound(keys.begin(), keys.end(), key);
    int index = it - keys.begin();
    
    // Insert key and right child
    keys.insert(it, key);
    children.insert(children.begin() + index + 1, right_child);
}

int InternalNode::serialize(char* buffer) const {
    int offset = 0;
    
    // Write node type (0 = internal)
    buffer[offset++] = 0;
    
    // Write number of keys
    int num_keys = keys.size();
    memcpy(buffer + offset, &num_keys, sizeof(int));
    offset += sizeof(int);
    
    // Write keys
    for (const auto& key : keys) {
        int key_len = key.length();
        memcpy(buffer + offset, &key_len, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer + offset, key.c_str(), key_len);
        offset += key_len;
    }
    
    // Write children page IDs
    for (int child : children) {
        memcpy(buffer + offset, &child, sizeof(int));
        offset += sizeof(int);
    }
    
    return offset;
}

int InternalNode::deserialize(const char* buffer) {
    int offset = 0;
    
    // Skip node type
    offset++;
    
    // Read number of keys
    int num_keys;
    memcpy(&num_keys, buffer + offset, sizeof(int));
    offset += sizeof(int);
    
    // Validate num_keys to prevent malicious/corrupted data
    if (num_keys < 0 || num_keys > 1000) {
        // Unreasonable number of keys (more than we could fit in a page)
        keys.clear();
        children.clear();
        return offset;
    }
    
    // Read keys
    keys.clear();
    for (int i = 0; i < num_keys; i++) {
        // Check if we have enough space left in buffer
        if (offset + sizeof(int) > PAGE_SIZE) {
            break;  // Corrupted data, stop reading
        }
        
        int key_len;
        memcpy(&key_len, buffer + offset, sizeof(int));
        offset += sizeof(int);
        
        // Validate key_len
        if (key_len < 0 || key_len > MAX_KEY_SIZE) {
            break;  // Invalid key length, stop reading
        }
        
        // Check if we have enough space for the key
        if (offset + key_len > PAGE_SIZE) {
            break;  // Not enough space, corrupted data
        }
        
        char key_buf[MAX_KEY_SIZE + 1];
        memcpy(key_buf, buffer + offset, key_len);
        key_buf[key_len] = '\0';
        offset += key_len;
        
        keys.push_back(std::string(key_buf));
    }
    
    // Read children page IDs
    children.clear();
    int expected_children = keys.size() + 1;  // Use actual keys read, not num_keys
    for (int i = 0; i < expected_children; i++) {
        // Check if we have enough space
        if (offset + sizeof(int) > PAGE_SIZE) {
            break;  // Corrupted data, stop reading
        }
        
        int child;
        memcpy(&child, buffer + offset, sizeof(int));
        offset += sizeof(int);
        children.push_back(child);
    }
    
    return offset;
}

bool InternalNode::needsSplit() const {
    // Estimate size: 1 (type) + 4 (num_keys) + keys + children
    // Conservative estimate: split if we have too many keys
    // With 64-byte keys, we can fit roughly 50-60 keys per node
    return keys.size() >= 56;
}

// ============================================================================
// LeafNode Implementation
// ============================================================================

LeafNode::LeafNode() : Node(true), next_leaf(-1) {}

int LeafNode::findEntry(const std::string& key) const {
    // Binary search for the entry
    int left = 0, right = entries.size() - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        std::string mid_key = entries[mid].getKey();
        if (mid_key == key) {
            return mid;
        } else if (mid_key < key) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

void LeafNode::insertEntry(const std::string& key, int value) {
    int index = findEntry(key);
    if (index != -1) {
        // Entry exists, add value
        entries[index].addValue(value);
    } else {
        // Create new entry
        LeafEntry new_entry(key, value);
        
        // Find insertion point to maintain sorted order
        auto it = std::lower_bound(entries.begin(), entries.end(), key,
            [](const LeafEntry& entry, const std::string& k) {
                return entry.getKey() < k;
            });
        entries.insert(it, new_entry);
    }
}

bool LeafNode::deleteEntry(const std::string& key, int value) {
    int index = findEntry(key);
    if (index == -1) {
        return false;  // Entry not found
    }
    
    bool removed = entries[index].removeValue(value);
    
    // If the entry has no more values, remove it
    if (removed && entries[index].values.empty()) {
        entries.erase(entries.begin() + index);
    }
    
    return removed;
}

std::vector<int> LeafNode::getValues(const std::string& key) const {
    int index = findEntry(key);
    if (index != -1) {
        return entries[index].values;
    }
    return std::vector<int>();
}

int LeafNode::serialize(char* buffer) const {
    int offset = 0;
    
    // Write node type (1 = leaf)
    buffer[offset++] = 1;
    
    // Write next_leaf
    memcpy(buffer + offset, &next_leaf, sizeof(int));
    offset += sizeof(int);
    
    // Write number of entries
    int num_entries = entries.size();
    memcpy(buffer + offset, &num_entries, sizeof(int));
    offset += sizeof(int);
    
    // Write each entry
    for (const auto& entry : entries) {
        // Write key
        int key_len = strlen(entry.key);
        memcpy(buffer + offset, &key_len, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer + offset, entry.key, key_len);
        offset += key_len;
        
        // Write number of values
        int num_values = entry.values.size();
        memcpy(buffer + offset, &num_values, sizeof(int));
        offset += sizeof(int);
        
        // Write values
        for (int value : entry.values) {
            memcpy(buffer + offset, &value, sizeof(int));
            offset += sizeof(int);
        }
    }
    
    return offset;
}

int LeafNode::deserialize(const char* buffer) {
    int offset = 0;
    
    // Skip node type
    offset++;
    
    // Check if we have enough space for next_leaf and num_entries
    if (offset + 2 * sizeof(int) > PAGE_SIZE) {
        entries.clear();
        next_leaf = -1;
        return offset;
    }
    
    // Read next_leaf
    memcpy(&next_leaf, buffer + offset, sizeof(int));
    offset += sizeof(int);
    
    // Read number of entries
    int num_entries;
    memcpy(&num_entries, buffer + offset, sizeof(int));
    offset += sizeof(int);
    
    // Validate num_entries to prevent malicious/corrupted data
    if (num_entries < 0 || num_entries > 1000) {
        // Unreasonable number of entries
        entries.clear();
        return offset;
    }
    
    // Read each entry
    entries.clear();
    for (int i = 0; i < num_entries; i++) {
        LeafEntry entry;
        
        // Check if we have enough space for key_len
        if (offset + sizeof(int) > PAGE_SIZE) {
            break;  // Corrupted data, stop reading
        }
        
        // Read key
        int key_len;
        memcpy(&key_len, buffer + offset, sizeof(int));
        offset += sizeof(int);
        
        // Validate key_len
        if (key_len < 0 || key_len > MAX_KEY_SIZE) {
            break;  // Invalid key length, stop reading
        }
        
        // Check if we have enough space for the key
        if (offset + key_len > PAGE_SIZE) {
            break;  // Not enough space, corrupted data
        }
        
        memcpy(entry.key, buffer + offset, key_len);
        entry.key[key_len] = '\0';
        offset += key_len;
        
        // Check if we have enough space for num_values
        if (offset + sizeof(int) > PAGE_SIZE) {
            break;  // Corrupted data, stop reading
        }
        
        // Read number of values
        int num_values;
        memcpy(&num_values, buffer + offset, sizeof(int));
        offset += sizeof(int);
        
        // Validate num_values
        if (num_values < 0 || num_values > 10000) {
            // Unreasonable number of values
            break;
        }
        
        // Read values
        for (int j = 0; j < num_values; j++) {
            // Check if we have enough space for the value
            if (offset + sizeof(int) > PAGE_SIZE) {
                break;  // Not enough space, stop reading values
            }
            
            int value;
            memcpy(&value, buffer + offset, sizeof(int));
            offset += sizeof(int);
            entry.values.push_back(value);
        }
        
        entries.push_back(entry);
    }
    
    return offset;
}

bool LeafNode::needsSplit() const {
    // Estimate serialized size
    int estimated_size = 1 + sizeof(int) * 2;  // type + next_leaf + num_entries
    for (const auto& entry : entries) {
        estimated_size += sizeof(int);  // key_len
        estimated_size += strlen(entry.key);
        estimated_size += sizeof(int);  // num_values
        estimated_size += entry.values.size() * sizeof(int);
    }
    
    // Leave some margin for safety
    return estimated_size > PAGE_SIZE - 100;
}

LeafNode* LeafNode::split() {
    LeafNode* new_node = new LeafNode();
    
    // Special case: single entry with many values
    // Split at VALUE level, not entry level, to avoid empty left node
    if (entries.size() == 1) {
        LeafEntry& entry = entries[0];
        int mid = entry.values.size() / 2;
        
        // Left entry: first half of values
        LeafEntry left_entry;
        memcpy(left_entry.key, entry.key, MAX_KEY_SIZE + 1);
        left_entry.values.assign(entry.values.begin(), entry.values.begin() + mid);
        
        // Right entry: second half of values
        LeafEntry right_entry;
        memcpy(right_entry.key, entry.key, MAX_KEY_SIZE + 1);
        right_entry.values.assign(entry.values.begin() + mid, entry.values.end());
        
        // Update this node's entry and add to new node
        entries[0] = left_entry;
        new_node->entries.push_back(right_entry);
    } else {
        // Normal case: multiple entries, split at entry level
        int mid = entries.size() / 2;
        
        // Move second half to new node
        new_node->entries.assign(entries.begin() + mid, entries.end());
        entries.erase(entries.begin() + mid, entries.end());
    }
    
    // Update next pointers
    new_node->next_leaf = this->next_leaf;
    // Note: this->next_leaf will be set to new_node's page_id by caller
    
    return new_node;
}

// ============================================================================
// Clone Methods (for cache support)
// ============================================================================

Node* InternalNode::clone() const {
    InternalNode* copy = new InternalNode();
    copy->page_id = this->page_id;
    copy->keys = this->keys;
    copy->children = this->children;
    return copy;
}

Node* LeafNode::clone() const {
    LeafNode* copy = new LeafNode();
    copy->page_id = this->page_id;
    copy->next_leaf = this->next_leaf;
    copy->entries = this->entries;
    return copy;
}

# B+ Tree Implementation Guide - Code Snippets

## Quick Reference for Implementation Team

This document provides ready-to-use code patterns for implementing the B+ Tree.

---

## 1. Core Data Structures

### Node Header Structure
```cpp
// Constants
const size_t NODE_SIZE = 4096;
const size_t HEADER_SIZE = 256;
const size_t MAX_KEY_SIZE = 64;

// Node types
enum NodeType : uint8_t {
    NODE_INTERNAL = 0x01,
    NODE_LEAF = 0x02
};

// File header (first 256 bytes of file)
struct FileHeader {
    uint32_t magic;           // 0x42504C54 ("BPLT")
    uint32_t version;         // 1
    uint64_t root_offset;     // Offset to root node
    uint64_t next_offset;     // Next free offset for new node
    uint32_t node_count;      // Total nodes allocated
    uint32_t tree_height;     // Current height
    uint32_t entry_count;     // Total entries in tree
    uint8_t  padding[224];    // Reserved for future use
    
    void serialize(char* buf) const {
        memcpy(buf, this, sizeof(FileHeader));
    }
    
    void deserialize(const char* buf) {
        memcpy(this, buf, sizeof(FileHeader));
    }
};
```

### Internal Node Structure
```cpp
struct InternalNode {
    static const size_t MAX_KEYS = 56;  // (4096-16) / (64+8)
    
    NodeType type;              // NODE_INTERNAL
    uint32_t key_count;         // Number of keys
    uint64_t parent_offset;     // Parent node offset (optional)
    
    char keys[MAX_KEYS][MAX_KEY_SIZE];
    uint64_t children[MAX_KEYS + 1];  // One more pointer than keys
    
    // Methods
    int find_child_index(const char* key) const {
        // Binary search
        int left = 0, right = key_count;
        while (left < right) {
            int mid = (left + right) / 2;
            if (strcmp(key, keys[mid]) < 0) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }
        return left;
    }
    
    bool is_full() const {
        return key_count >= MAX_KEYS;
    }
    
    void serialize(char* buf) const {
        size_t offset = 0;
        memcpy(buf + offset, &type, sizeof(type)); offset += sizeof(type);
        memcpy(buf + offset, &key_count, sizeof(key_count)); offset += sizeof(key_count);
        memcpy(buf + offset, &parent_offset, sizeof(parent_offset)); offset += sizeof(parent_offset);
        
        // Padding to align to 16 bytes
        offset = 16;
        
        for (size_t i = 0; i < key_count; i++) {
            memcpy(buf + offset, keys[i], MAX_KEY_SIZE);
            offset += MAX_KEY_SIZE;
            memcpy(buf + offset, &children[i], sizeof(uint64_t));
            offset += sizeof(uint64_t);
        }
        // Last child pointer
        memcpy(buf + offset, &children[key_count], sizeof(uint64_t));
    }
    
    void deserialize(const char* buf) {
        size_t offset = 0;
        memcpy(&type, buf + offset, sizeof(type)); offset += sizeof(type);
        memcpy(&key_count, buf + offset, sizeof(key_count)); offset += sizeof(key_count);
        memcpy(&parent_offset, buf + offset, sizeof(parent_offset)); offset += sizeof(parent_offset);
        
        offset = 16;
        
        for (size_t i = 0; i < key_count; i++) {
            memcpy(keys[i], buf + offset, MAX_KEY_SIZE);
            offset += MAX_KEY_SIZE;
            memcpy(&children[i], buf + offset, sizeof(uint64_t));
            offset += sizeof(uint64_t);
        }
        memcpy(&children[key_count], buf + offset, sizeof(uint64_t));
    }
};
```

### Leaf Node Structure
```cpp
struct LeafEntry {
    char key[MAX_KEY_SIZE];
    std::vector<int> values;  // Sorted values
    
    LeafEntry() {
        memset(key, 0, MAX_KEY_SIZE);
    }
    
    void add_value(int value) {
        auto it = std::lower_bound(values.begin(), values.end(), value);
        if (it == values.end() || *it != value) {
            values.insert(it, value);
        }
    }
    
    bool remove_value(int value) {
        auto it = std::find(values.begin(), values.end(), value);
        if (it != values.end()) {
            values.erase(it);
            return true;
        }
        return false;
    }
};

struct LeafNode {
    static const size_t MAX_ENTRIES = 50;  // Conservative estimate
    
    NodeType type;              // NODE_LEAF
    uint32_t entry_count;       // Number of entries
    uint64_t next_leaf;         // Next leaf in linked list
    
    LeafEntry entries[MAX_ENTRIES];
    
    int find_entry(const char* key) const {
        // Binary search
        int left = 0, right = entry_count - 1;
        while (left <= right) {
            int mid = (left + right) / 2;
            int cmp = strcmp(key, entries[mid].key);
            if (cmp == 0) {
                return mid;
            } else if (cmp < 0) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }
        return -1;  // Not found
    }
    
    int find_insert_position(const char* key) const {
        int left = 0, right = entry_count;
        while (left < right) {
            int mid = (left + right) / 2;
            if (strcmp(key, entries[mid].key) < 0) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }
        return left;
    }
    
    bool is_full() const {
        return entry_count >= MAX_ENTRIES;
    }
    
    // Serialization is more complex due to variable-length values
    void serialize(char* buf) const {
        size_t offset = 0;
        memcpy(buf + offset, &type, sizeof(type)); offset += sizeof(type);
        memcpy(buf + offset, &entry_count, sizeof(entry_count)); offset += sizeof(entry_count);
        memcpy(buf + offset, &next_leaf, sizeof(next_leaf)); offset += sizeof(next_leaf);
        
        offset = 16;  // Align
        
        for (size_t i = 0; i < entry_count; i++) {
            // Write key
            memcpy(buf + offset, entries[i].key, MAX_KEY_SIZE);
            offset += MAX_KEY_SIZE;
            
            // Write value count
            uint32_t value_count = entries[i].values.size();
            memcpy(buf + offset, &value_count, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            
            // Write values
            for (int value : entries[i].values) {
                memcpy(buf + offset, &value, sizeof(int));
                offset += sizeof(int);
            }
        }
    }
    
    void deserialize(const char* buf) {
        size_t offset = 0;
        memcpy(&type, buf + offset, sizeof(type)); offset += sizeof(type);
        memcpy(&entry_count, buf + offset, sizeof(entry_count)); offset += sizeof(entry_count);
        memcpy(&next_leaf, buf + offset, sizeof(next_leaf)); offset += sizeof(next_leaf);
        
        offset = 16;
        
        for (size_t i = 0; i < entry_count; i++) {
            // Read key
            memcpy(entries[i].key, buf + offset, MAX_KEY_SIZE);
            offset += MAX_KEY_SIZE;
            
            // Read value count
            uint32_t value_count;
            memcpy(&value_count, buf + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            
            // Read values
            entries[i].values.clear();
            for (size_t j = 0; j < value_count; j++) {
                int value;
                memcpy(&value, buf + offset, sizeof(int));
                offset += sizeof(int);
                entries[i].values.push_back(value);
            }
        }
    }
};
```

---

## 2. File I/O Layer

```cpp
class FileManager {
private:
    FILE* fp;
    FileHeader header;
    
public:
    FileManager(const char* filename) {
        bool exists = (access(filename, F_OK) == 0);
        
        if (exists) {
            fp = fopen(filename, "rb+");
            load_header();
        } else {
            fp = fopen(filename, "wb+");
            initialize_header();
        }
        
        // Set larger buffer for better I/O performance
        static char io_buffer[64 * 1024];
        setvbuf(fp, io_buffer, _IOFBF, sizeof(io_buffer));
    }
    
    ~FileManager() {
        save_header();
        fclose(fp);
    }
    
    void load_header() {
        char buf[HEADER_SIZE];
        fseek(fp, 0, SEEK_SET);
        fread(buf, HEADER_SIZE, 1, fp);
        header.deserialize(buf);
    }
    
    void save_header() {
        char buf[HEADER_SIZE];
        memset(buf, 0, HEADER_SIZE);
        header.serialize(buf);
        fseek(fp, 0, SEEK_SET);
        fwrite(buf, HEADER_SIZE, 1, fp);
        fflush(fp);
    }
    
    void initialize_header() {
        header.magic = 0x42504C54;
        header.version = 1;
        header.root_offset = 0;  // 0 means empty tree
        header.next_offset = HEADER_SIZE;
        header.node_count = 0;
        header.tree_height = 0;
        header.entry_count = 0;
        save_header();
    }
    
    uint64_t allocate_node() {
        uint64_t offset = header.next_offset;
        header.next_offset += NODE_SIZE;
        header.node_count++;
        return offset;
    }
    
    void read_node(uint64_t offset, char* buf) {
        fseek(fp, offset, SEEK_SET);
        fread(buf, NODE_SIZE, 1, fp);
    }
    
    void write_node(uint64_t offset, const char* buf) {
        fseek(fp, offset, SEEK_SET);
        fwrite(buf, NODE_SIZE, 1, fp);
    }
    
    uint64_t get_root_offset() const { return header.root_offset; }
    void set_root_offset(uint64_t offset) { header.root_offset = offset; }
};
```

---

## 3. Node Cache (LRU)

```cpp
class NodeCache {
private:
    struct CacheEntry {
        char* data;
        bool dirty;
        
        CacheEntry() : data(new char[NODE_SIZE]), dirty(false) {}
        ~CacheEntry() { delete[] data; }
    };
    
    static const size_t MAX_CACHE_SIZE = 1000;
    
    std::unordered_map<uint64_t, std::shared_ptr<CacheEntry>> cache;
    std::list<uint64_t> lru_list;
    std::unordered_map<uint64_t, std::list<uint64_t>::iterator> lru_map;
    
    FileManager* file_mgr;
    
public:
    NodeCache(FileManager* fm) : file_mgr(fm) {}
    
    ~NodeCache() {
        flush_all();
    }
    
    char* get(uint64_t offset) {
        auto it = cache.find(offset);
        if (it != cache.end()) {
            // Move to front
            lru_list.erase(lru_map[offset]);
            lru_list.push_front(offset);
            lru_map[offset] = lru_list.begin();
            
            return it->second->data;
        }
        
        // Load from disk
        auto entry = std::make_shared<CacheEntry>();
        file_mgr->read_node(offset, entry->data);
        
        // Evict if necessary
        if (cache.size() >= MAX_CACHE_SIZE) {
            evict_lru();
        }
        
        // Add to cache
        cache[offset] = entry;
        lru_list.push_front(offset);
        lru_map[offset] = lru_list.begin();
        
        return entry->data;
    }
    
    void mark_dirty(uint64_t offset) {
        auto it = cache.find(offset);
        if (it != cache.end()) {
            it->second->dirty = true;
        }
    }
    
    void evict_lru() {
        if (lru_list.empty()) return;
        
        uint64_t evict_offset = lru_list.back();
        lru_list.pop_back();
        lru_map.erase(evict_offset);
        
        auto entry = cache[evict_offset];
        if (entry->dirty) {
            file_mgr->write_node(evict_offset, entry->data);
        }
        
        cache.erase(evict_offset);
    }
    
    void flush_all() {
        for (auto& [offset, entry] : cache) {
            if (entry->dirty) {
                file_mgr->write_node(offset, entry->data);
                entry->dirty = false;
            }
        }
    }
};
```

---

## 4. Main B+ Tree Class

```cpp
class BPlusTree {
private:
    FileManager file_mgr;
    NodeCache cache;
    
public:
    BPlusTree(const char* filename) 
        : file_mgr(filename), cache(&file_mgr) {}
    
    ~BPlusTree() {
        cache.flush_all();
    }
    
    void insert(const char* key, int value) {
        uint64_t root_offset = file_mgr.get_root_offset();
        
        if (root_offset == 0) {
            // Create initial root (leaf)
            root_offset = create_initial_leaf();
            file_mgr.set_root_offset(root_offset);
        }
        
        // Find leaf to insert
        uint64_t leaf_offset = find_leaf(root_offset, key);
        char* node_data = cache.get(leaf_offset);
        LeafNode leaf;
        leaf.deserialize(node_data);
        
        // Insert into leaf
        int idx = leaf.find_entry(key);
        if (idx >= 0) {
            // Key exists, add value
            leaf.entries[idx].add_value(value);
        } else {
            // New key
            idx = leaf.find_insert_position(key);
            // Shift entries
            for (int i = leaf.entry_count; i > idx; i--) {
                leaf.entries[i] = leaf.entries[i-1];
            }
            strncpy(leaf.entries[idx].key, key, MAX_KEY_SIZE);
            leaf.entries[idx].values.clear();
            leaf.entries[idx].values.push_back(value);
            leaf.entry_count++;
        }
        
        // Check if split needed
        if (leaf.is_full()) {
            split_leaf(leaf_offset, leaf);
        } else {
            leaf.serialize(node_data);
            cache.mark_dirty(leaf_offset);
        }
    }
    
    std::vector<int> find(const char* key) {
        uint64_t root_offset = file_mgr.get_root_offset();
        if (root_offset == 0) {
            return {};  // Empty tree
        }
        
        uint64_t leaf_offset = find_leaf(root_offset, key);
        char* node_data = cache.get(leaf_offset);
        LeafNode leaf;
        leaf.deserialize(node_data);
        
        int idx = leaf.find_entry(key);
        if (idx >= 0) {
            return leaf.entries[idx].values;
        }
        return {};
    }
    
    void remove(const char* key, int value) {
        uint64_t root_offset = file_mgr.get_root_offset();
        if (root_offset == 0) {
            return;  // Empty tree
        }
        
        uint64_t leaf_offset = find_leaf(root_offset, key);
        char* node_data = cache.get(leaf_offset);
        LeafNode leaf;
        leaf.deserialize(node_data);
        
        int idx = leaf.find_entry(key);
        if (idx >= 0) {
            if (leaf.entries[idx].remove_value(value)) {
                if (leaf.entries[idx].values.empty()) {
                    // Remove entire entry
                    for (size_t i = idx; i < leaf.entry_count - 1; i++) {
                        leaf.entries[i] = leaf.entries[i+1];
                    }
                    leaf.entry_count--;
                }
                leaf.serialize(node_data);
                cache.mark_dirty(leaf_offset);
            }
        }
        // If not found, no-op (per problem requirement)
    }
    
private:
    uint64_t create_initial_leaf() {
        uint64_t offset = file_mgr.allocate_node();
        LeafNode leaf;
        leaf.type = NODE_LEAF;
        leaf.entry_count = 0;
        leaf.next_leaf = 0;
        
        char* node_data = cache.get(offset);
        leaf.serialize(node_data);
        cache.mark_dirty(offset);
        
        return offset;
    }
    
    uint64_t find_leaf(uint64_t start_offset, const char* key) {
        uint64_t current = start_offset;
        
        while (true) {
            char* node_data = cache.get(current);
            NodeType type = *reinterpret_cast<NodeType*>(node_data);
            
            if (type == NODE_LEAF) {
                return current;
            }
            
            // Internal node
            InternalNode internal;
            internal.deserialize(node_data);
            int idx = internal.find_child_index(key);
            current = internal.children[idx];
        }
    }
    
    void split_leaf(uint64_t old_offset, LeafNode& old_leaf) {
        // Create new leaf
        uint64_t new_offset = file_mgr.allocate_node();
        LeafNode new_leaf;
        new_leaf.type = NODE_LEAF;
        new_leaf.entry_count = 0;
        new_leaf.next_leaf = old_leaf.next_leaf;
        
        // Split entries
        size_t mid = old_leaf.entry_count / 2;
        for (size_t i = mid; i < old_leaf.entry_count; i++) {
            new_leaf.entries[new_leaf.entry_count++] = old_leaf.entries[i];
        }
        old_leaf.entry_count = mid;
        old_leaf.next_leaf = new_offset;
        
        // Write both leaves
        char* old_data = cache.get(old_offset);
        old_leaf.serialize(old_data);
        cache.mark_dirty(old_offset);
        
        char* new_data = cache.get(new_offset);
        new_leaf.serialize(new_data);
        cache.mark_dirty(new_offset);
        
        // Promote key to parent (simplified - assumes single root)
        // In full implementation, would need to handle parent updates
        // and potential parent splits
    }
};
```

---

## 5. Main Program Template

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

int main() {
    BPlusTree tree("data.bpt");
    
    int n;
    std::cin >> n;
    
    std::string cmd;
    for (int i = 0; i < n; i++) {
        std::cin >> cmd;
        
        if (cmd == "insert") {
            std::string index;
            int value;
            std::cin >> index >> value;
            tree.insert(index.c_str(), value);
            
        } else if (cmd == "delete") {
            std::string index;
            int value;
            std::cin >> index >> value;
            tree.remove(index.c_str(), value);
            
        } else if (cmd == "find") {
            std::string index;
            std::cin >> index;
            
            auto values = tree.find(index.c_str());
            if (values.empty()) {
                std::cout << "null\n";
            } else {
                for (size_t i = 0; i < values.size(); i++) {
                    if (i > 0) std::cout << " ";
                    std::cout << values[i];
                }
                std::cout << "\n";
            }
        }
    }
    
    return 0;
}
```

---

## 6. Build System

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.10)
project(BPlusTree)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

# Source files
add_executable(code
    main.cpp
    # Add other source files here
)

# Link math library if needed
target_link_libraries(code m)
```

### Makefile (alternative)
```makefile
CXX = g++-13
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
TARGET = code

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
```

### .gitignore
```gitignore
# Build artifacts
code
*.o
*.out
CMakeFiles/
CMakeCache.txt
cmake_install.cmake
Makefile

# Data files (test artifacts)
*.bpt
*.dat
*.idx

# IDE
.vscode/
.idea/
*.swp
```

---

## 7. Testing Checklist

- [ ] Empty tree find returns null
- [ ] Single insert and find
- [ ] Multiple values for same key
- [ ] Values returned in sorted order
- [ ] Delete non-existent entry (no-op)
- [ ] Delete with value that doesn't match key (no-op)
- [ ] Sample input from problem
- [ ] Large dataset (300K operations)
- [ ] File persistence across runs
- [ ] Memory usage under 64 MiB
- [ ] Execution time under 5 seconds

---

## 8. Common Pitfalls to Avoid

1. **String handling**: Always null-terminate and use strncpy
2. **Binary search bounds**: Careful with < vs <=
3. **File pointer**: Always fseek before read/write
4. **Cache coherency**: Mark dirty after modifications
5. **Split logic**: Don't forget to update parent links
6. **Serialization**: Ensure consistent byte ordering
7. **Value sorting**: Keep values sorted at all times
8. **Empty tree**: Handle root_offset == 0 case

---

This guide provides the core building blocks. The implementation team should:

1. Start with file I/O and basic node structures
2. Implement find operation first (simpler)
3. Add insert without splitting
4. Add splitting logic
5. Add delete (simplified, no merging)
6. Optimize with caching
7. Test thoroughly with sample and edge cases

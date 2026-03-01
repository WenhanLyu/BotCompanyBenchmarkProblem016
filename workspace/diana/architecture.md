# B+ Tree File Storage Database - System Architecture

**Project**: File Storage BPT (ACMOJ Problem 2697)  
**Architect**: Diana  
**Date**: 2024  
**Version**: 1.0

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [System Overview](#system-overview)
3. [Architecture Diagram](#architecture-diagram)
4. [File Format Specification](#file-format-specification)
5. [Data Structures](#data-structures)
6. [Component Responsibilities](#component-responsibilities)
7. [Memory Management Strategy](#memory-management-strategy)
8. [Performance Characteristics](#performance-characteristics)
9. [Implementation Milestones](#implementation-milestones)
10. [Testing Strategy](#testing-strategy)
11. [Risk Analysis](#risk-analysis)

---

## 1. Executive Summary

This document specifies the architecture for a file-based B+ Tree key-value database that supports:
- Insert, delete, and find operations on (string key, integer value) pairs
- Multiple values per key (multi-map semantics)
- Persistent storage across program runs
- Operations on 300,000+ entries within strict resource limits

**Key Design Decisions:**
- Single file storage with 4KB fixed-size nodes
- B+ Tree with internal nodes and linked leaf nodes
- LRU cache for hot nodes (root and upper levels)
- Simplified deletion (no node merging)
- Inline value storage in leaf nodes with sorted vectors

**Resource Targets:**
- Memory: < 16 MiB (limit: 64 MiB)
- Disk: < 500 MiB (limit: 1024 MiB)
- Time: < 3 seconds for 300K operations (limit: 5 seconds)
- Files: 1 file (limit: 20 files)

---

## 2. System Overview

### 2.1 High-Level Architecture

```
┌──────────────────────────────────────────────────────────┐
│                    Main Program                          │
│  (Command Parser + I/O Handler)                         │
└────────────────┬─────────────────────────────────────────┘
                 │
                 ▼
┌──────────────────────────────────────────────────────────┐
│                   BPlusTree Class                        │
│  • insert(key, value)                                    │
│  • find(key) → vector<int>                              │
│  • delete(key, value)                                    │
└────────────┬──────────────┬──────────────────────────────┘
             │              │
             ▼              ▼
    ┌────────────────┐  ┌──────────────────┐
    │  Node Cache    │  │  File Manager    │
    │  (LRU)         │  │  (I/O Layer)     │
    └────────┬───────┘  └────────┬─────────┘
             │                   │
             └───────┬───────────┘
                     ▼
             ┌───────────────────┐
             │  data.bpt (file)  │
             │  • File Header    │
             │  • Internal Nodes │
             │  • Leaf Nodes     │
             └───────────────────┘
```

### 2.2 Data Flow

**Insert Operation:**
```
User Input → Parser → BPlusTree::insert() 
  → find_leaf() [via cache] 
  → modify leaf node 
  → split if needed 
  → update parent chain 
  → mark dirty → cache flush
```

**Find Operation:**
```
User Input → Parser → BPlusTree::find()
  → find_leaf() [via cache]
  → binary search in leaf
  → return sorted values → output
```

**Delete Operation:**
```
User Input → Parser → BPlusTree::delete()
  → find_leaf() [via cache]
  → remove value from entry
  → remove entry if empty
  → mark dirty → cache flush
```

---

## 3. Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                         FILE LAYOUT (data.bpt)                      │
├─────────────────────────────────────────────────────────────────────┤
│ Offset 0: File Header (256 bytes)                                  │
│   ├─ magic: 0x42504C54 ("BPLT")                                    │
│   ├─ version: 1                                                     │
│   ├─ root_offset: uint64_t                                          │
│   ├─ next_offset: uint64_t (next free space)                       │
│   ├─ node_count: uint32_t                                           │
│   ├─ tree_height: uint32_t                                          │
│   ├─ entry_count: uint32_t                                          │
│   └─ reserved: 224 bytes                                            │
├─────────────────────────────────────────────────────────────────────┤
│ Offset 256: Node 0 (4096 bytes)                                    │
├─────────────────────────────────────────────────────────────────────┤
│ Offset 4352: Node 1 (4096 bytes)                                   │
├─────────────────────────────────────────────────────────────────────┤
│ ...                                                                 │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                    INTERNAL NODE STRUCTURE (4096 bytes)             │
├─────────────────────────────────────────────────────────────────────┤
│ Header (16 bytes):                                                  │
│   ├─ type: NodeType (1 byte) = NODE_INTERNAL                       │
│   ├─ padding: 3 bytes                                               │
│   ├─ key_count: uint32_t (4 bytes)                                 │
│   └─ parent_offset: uint64_t (8 bytes)                             │
├─────────────────────────────────────────────────────────────────────┤
│ Keys and Children:                                                  │
│   For i = 0 to key_count-1:                                        │
│     ├─ key[i]: char[64]                                            │
│     └─ child[i]: uint64_t (offset)                                 │
│   └─ child[key_count]: uint64_t (rightmost child)                 │
├─────────────────────────────────────────────────────────────────────┤
│ Capacity: ~56 keys, 57 children                                    │
│   (4096 - 16) / (64 + 8) ≈ 56                                      │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                      LEAF NODE STRUCTURE (4096 bytes)               │
├─────────────────────────────────────────────────────────────────────┤
│ Header (16 bytes):                                                  │
│   ├─ type: NodeType (1 byte) = NODE_LEAF                           │
│   ├─ padding: 3 bytes                                               │
│   ├─ entry_count: uint32_t (4 bytes)                               │
│   └─ next_leaf: uint64_t (8 bytes, linked list)                    │
├─────────────────────────────────────────────────────────────────────┤
│ Variable-length entries:                                            │
│   For each entry:                                                   │
│     ├─ key: char[64]                                               │
│     ├─ value_count: uint32_t                                       │
│     └─ values[]: int[value_count] (sorted)                         │
├─────────────────────────────────────────────────────────────────────┤
│ Capacity: ~50 entries (conservative, depends on value count)       │
│   Worst case: 50 * (64 + 4 + 4) = 3600 bytes                       │
│   Average case: allows 2-3 values per key                          │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                        B+ TREE STRUCTURE                            │
├─────────────────────────────────────────────────────────────────────┤
│                          [Root Internal]                            │
│                         /      |       \                            │
│                 [Internal]  [Internal]  [Internal]                  │
│                 /    |   \                                          │
│            [Leaf]→[Leaf]→[Leaf]→[Leaf]→...                         │
│                                                                     │
│ Properties:                                                         │
│  • All leaves at same level                                        │
│  • Leaves linked for range queries                                 │
│  • Internal nodes guide search                                     │
│  • Height = O(log_56 N) ≈ 3-4 for N=300K                           │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 4. File Format Specification

### 4.1 File Header (Offset 0, 256 bytes)

```cpp
struct FileHeader {
    // Identification (8 bytes)
    uint32_t magic;           // 0x42504C54 ("BPLT" - B+ Leaf Tree)
    uint32_t version;         // Format version = 1
    
    // Tree metadata (24 bytes)
    uint64_t root_offset;     // Offset to root node (0 = empty tree)
    uint64_t next_offset;     // Next free offset for allocation
    uint32_t node_count;      // Total nodes allocated
    uint32_t tree_height;     // Current tree height (0 = empty)
    
    // Statistics (8 bytes)
    uint32_t entry_count;     // Total key-value pairs
    uint32_t reserved1;       // Reserved for future use
    
    // Reserved space (224 bytes)
    uint8_t padding[224];     // Future extensions
    
    // Total: 256 bytes
};
```

**Design Rationale:**
- 256-byte header allows alignment and future expansion
- `magic` number for file format validation
- `root_offset = 0` indicates empty tree (no special sentinel)
- `next_offset` implements simple bump allocator
- Reserved space allows adding features without breaking compatibility

### 4.2 Node Layout (4096 bytes each)

**Common Header (16 bytes):**
```cpp
struct NodeHeader {
    NodeType type;           // 1 byte: NODE_INTERNAL or NODE_LEAF
    uint8_t padding[3];      // Alignment
    uint32_t count;          // key_count or entry_count
    uint64_t link;           // parent_offset or next_leaf
    // Total: 16 bytes
};
```

**Internal Node (4096 bytes):**
```
[Header: 16 bytes]
[Key 0: 64 bytes][Child 0: 8 bytes]
[Key 1: 64 bytes][Child 1: 8 bytes]
...
[Key N-1: 64 bytes][Child N-1: 8 bytes]
[Child N: 8 bytes]  // Rightmost child
[Unused space]

Capacity: (4096 - 16 - 8) / 72 = 56 keys, 57 children
```

**Leaf Node (4096 bytes):**
```
[Header: 16 bytes]
[Entry 0]
  [Key: 64 bytes]
  [Value Count: 4 bytes]
  [Value 0: 4 bytes]
  [Value 1: 4 bytes]
  ...
[Entry 1]
  [Key: 64 bytes]
  [Value Count: 4 bytes]
  [Values...]
...
[Unused space]

Variable size entries. Conservative limit: 50 entries
```

### 4.3 Key Format

- **Type**: Fixed-size char array
- **Size**: 64 bytes (matches problem constraint)
- **Encoding**: Null-terminated C string
- **Comparison**: `strcmp()` for lexicographic ordering
- **Padding**: Unused bytes set to 0

### 4.4 Value Format

- **Type**: `int` (4 bytes, two's complement)
- **Range**: -2,147,483,648 to 2,147,483,647
- **Storage**: Direct binary (no serialization overhead)
- **Ordering**: Sorted array within each leaf entry

---

## 5. Data Structures

### 5.1 Core Classes

```cpp
// Node type enumeration
enum NodeType : uint8_t {
    NODE_INTERNAL = 0x01,
    NODE_LEAF     = 0x02
};

// File header
struct FileHeader {
    uint32_t magic;
    uint32_t version;
    uint64_t root_offset;
    uint64_t next_offset;
    uint32_t node_count;
    uint32_t tree_height;
    uint32_t entry_count;
    uint8_t  padding[224];
    
    void serialize(char* buf) const;
    void deserialize(const char* buf);
};

// Internal node (in-memory representation)
struct InternalNode {
    static const size_t MAX_KEYS = 56;
    
    NodeType type;
    uint32_t key_count;
    uint64_t parent_offset;
    
    char keys[MAX_KEYS][64];
    uint64_t children[MAX_KEYS + 1];
    
    int find_child_index(const char* key) const;
    bool is_full() const { return key_count >= MAX_KEYS; }
    void insert_key(int index, const char* key, uint64_t left_child, uint64_t right_child);
    void serialize(char* buf) const;
    void deserialize(const char* buf);
};

// Leaf entry (in-memory representation)
struct LeafEntry {
    char key[64];
    std::vector<int> values;  // Sorted
    
    void add_value(int value);
    bool remove_value(int value);
};

// Leaf node (in-memory representation)
struct LeafNode {
    static const size_t MAX_ENTRIES = 50;
    
    NodeType type;
    uint32_t entry_count;
    uint64_t next_leaf;
    
    LeafEntry entries[MAX_ENTRIES];
    
    int find_entry(const char* key) const;
    int find_insert_position(const char* key) const;
    bool is_full() const { return entry_count >= MAX_ENTRIES; }
    void serialize(char* buf) const;
    void deserialize(const char* buf);
};

// File manager (I/O layer)
class FileManager {
private:
    FILE* fp;
    FileHeader header;
    
public:
    FileManager(const char* filename);
    ~FileManager();
    
    void load_header();
    void save_header();
    void initialize_header();
    
    uint64_t allocate_node();
    void read_node(uint64_t offset, char* buf);
    void write_node(uint64_t offset, const char* buf);
    
    uint64_t get_root_offset() const;
    void set_root_offset(uint64_t offset);
};

// Node cache (LRU)
class NodeCache {
private:
    struct CacheEntry {
        char* data;
        bool dirty;
    };
    
    static const size_t MAX_CACHE_SIZE = 1000;
    
    std::unordered_map<uint64_t, std::shared_ptr<CacheEntry>> cache;
    std::list<uint64_t> lru_list;
    std::unordered_map<uint64_t, std::list<uint64_t>::iterator> lru_map;
    
    FileManager* file_mgr;
    
public:
    NodeCache(FileManager* fm);
    ~NodeCache();
    
    char* get(uint64_t offset);
    void mark_dirty(uint64_t offset);
    void evict_lru();
    void flush_all();
};

// Main B+ Tree
class BPlusTree {
private:
    FileManager file_mgr;
    NodeCache cache;
    
    uint64_t create_initial_leaf();
    uint64_t find_leaf(uint64_t start_offset, const char* key);
    void split_leaf(uint64_t leaf_offset, LeafNode& leaf);
    void split_internal(uint64_t node_offset, InternalNode& node);
    void insert_into_parent(uint64_t left_offset, const char* key, uint64_t right_offset);
    
public:
    BPlusTree(const char* filename);
    ~BPlusTree();
    
    void insert(const char* key, int value);
    std::vector<int> find(const char* key);
    void remove(const char* key, int value);
};
```

### 5.2 Serialization Strategy

**Internal Node Serialization:**
```cpp
void InternalNode::serialize(char* buf) const {
    size_t offset = 0;
    
    // Header (16 bytes)
    *reinterpret_cast<NodeType*>(buf + offset) = type;
    offset += sizeof(NodeType);
    offset += 3; // padding
    *reinterpret_cast<uint32_t*>(buf + offset) = key_count;
    offset += sizeof(uint32_t);
    *reinterpret_cast<uint64_t*>(buf + offset) = parent_offset;
    offset = 16;
    
    // Keys and children (interleaved)
    for (size_t i = 0; i < key_count; i++) {
        memcpy(buf + offset, keys[i], 64);
        offset += 64;
        *reinterpret_cast<uint64_t*>(buf + offset) = children[i];
        offset += 8;
    }
    
    // Last child
    *reinterpret_cast<uint64_t*>(buf + offset) = children[key_count];
}
```

**Leaf Node Serialization:**
```cpp
void LeafNode::serialize(char* buf) const {
    size_t offset = 0;
    
    // Header (16 bytes)
    *reinterpret_cast<NodeType*>(buf + offset) = type;
    offset += sizeof(NodeType);
    offset += 3; // padding
    *reinterpret_cast<uint32_t*>(buf + offset) = entry_count;
    offset += sizeof(uint32_t);
    *reinterpret_cast<uint64_t*>(buf + offset) = next_leaf;
    offset = 16;
    
    // Variable-length entries
    for (size_t i = 0; i < entry_count; i++) {
        // Key
        memcpy(buf + offset, entries[i].key, 64);
        offset += 64;
        
        // Value count
        uint32_t value_count = entries[i].values.size();
        *reinterpret_cast<uint32_t*>(buf + offset) = value_count;
        offset += 4;
        
        // Values
        for (int value : entries[i].values) {
            *reinterpret_cast<int*>(buf + offset) = value;
            offset += 4;
        }
    }
    
    // Zero out rest of node
    memset(buf + offset, 0, 4096 - offset);
}
```

---

## 6. Component Responsibilities

### 6.1 Main Program (`main.cpp`)

**Responsibilities:**
- Parse command-line input (n commands)
- Dispatch to BPlusTree methods
- Format and output results
- Handle I/O errors gracefully

**Interface:**
```cpp
int main() {
    BPlusTree tree("data.bpt");
    
    int n;
    cin >> n;
    
    for (int i = 0; i < n; i++) {
        string cmd;
        cin >> cmd;
        
        if (cmd == "insert") {
            string key;
            int value;
            cin >> key >> value;
            tree.insert(key.c_str(), value);
        }
        else if (cmd == "find") {
            string key;
            cin >> key;
            vector<int> results = tree.find(key.c_str());
            
            if (results.empty()) {
                cout << "null\n";
            } else {
                for (size_t i = 0; i < results.size(); i++) {
                    if (i > 0) cout << " ";
                    cout << results[i];
                }
                cout << "\n";
            }
        }
        else if (cmd == "delete") {
            string key;
            int value;
            cin >> key >> value;
            tree.remove(key.c_str(), value);
        }
    }
    
    return 0;
}
```

### 6.2 BPlusTree Class

**Responsibilities:**
- Maintain B+ Tree invariants
- Coordinate insert/find/delete operations
- Handle node splitting and parent updates
- Ensure persistence via cache

**Key Algorithms:**
- `insert()`: Find leaf → insert value → split if needed → update parents
- `find()`: Traverse from root to leaf → binary search → return values
- `delete()`: Find leaf → remove value → remove entry if empty (no merge)
- `split_leaf()`: Create new leaf → move half entries → insert key to parent
- `split_internal()`: Create new internal → move half keys → insert to parent

### 6.3 FileManager Class

**Responsibilities:**
- Abstract file I/O operations
- Manage file header
- Allocate node offsets
- Read/write raw node data

**Guarantees:**
- File is created if it doesn't exist
- Header is loaded on open, saved on close
- Node allocations are sequential (no fragmentation)
- Buffered I/O for performance

### 6.4 NodeCache Class

**Responsibilities:**
- Cache hot nodes in memory (LRU eviction)
- Track dirty nodes
- Flush dirty nodes on eviction/close
- Provide transparent access to nodes

**Performance:**
- Root node always cached (never evicted)
- Upper levels stay hot (high hit rate)
- Reduces I/O by 90-95%
- Memory bounded: 1000 nodes * 4KB = 4MB

---

## 7. Memory Management Strategy

### 7.1 Memory Budget (64 MiB limit)

| Component             | Size         | Notes                               |
|-----------------------|--------------|-------------------------------------|
| Code + Stack          | ~2 MiB       | Compiled binary + runtime stack     |
| FileManager           | ~256 bytes   | Header + file handle                |
| NodeCache (1000)      | ~4 MiB       | 1000 nodes * 4KB                    |
| Cache metadata        | ~100 KB      | Hash maps + LRU list                |
| BPlusTree state       | ~1 KB        | Tree state variables                |
| I/O buffers           | ~64 KB       | File buffering                      |
| Temporary objects     | ~1 MiB       | In-memory nodes during operations   |
| **Total**             | **~8 MiB**   | Well under 64 MiB limit             |

### 7.2 Memory Management Techniques

**Static Allocation:**
- Node buffer: `char node_buf[4096]` on stack
- Fixed-size arrays in structs
- No dynamic growth of core structures

**Cache Management:**
- Fixed-size cache (1000 nodes)
- LRU eviction when full
- Dirty flag to minimize writes

**Smart Object Lifetime:**
- In-memory nodes (InternalNode, LeafNode) are temporary
- Only serialized form stored in cache
- Deserialized on-demand, discarded after use

**Avoiding Fragmentation:**
- Use `std::vector` for values (grows geometrically)
- Pre-allocate cache entries
- No global containers that grow unbounded

---

## 8. Performance Characteristics

### 8.1 Time Complexity

| Operation | Best Case | Average Case | Worst Case | Notes                    |
|-----------|-----------|--------------|------------|--------------------------|
| Insert    | O(1)      | O(log N)     | O(log N)   | Cache hit vs miss        |
| Find      | O(1)      | O(log N)     | O(log N)   | Cache hit vs miss        |
| Delete    | O(1)      | O(log N)     | O(log N)   | Cache hit vs miss        |

**Tree Height:** h = ⌈log₅₆(N/50)⌉ + 1
- For N = 300,000 entries: h ≈ 3-4
- Each level requires 1 I/O (if not cached)

### 8.2 Space Complexity

| Component      | Disk Usage                | Memory Usage          |
|----------------|---------------------------|-----------------------|
| File Header    | 256 bytes                 | 256 bytes             |
| Nodes          | N_nodes * 4096 bytes      | 1000 * 4096 (cache)   |
| **Total**      | ~300 MB (for 300K ops)    | ~4 MB (cache)         |

**Disk Usage Estimate:**
- 300K entries / 50 entries per leaf = 6,000 leaf nodes
- Internal nodes (3 levels): ~107 + 2 + 1 = 110 nodes
- Total: ~6,110 nodes * 4KB = ~24.4 MB

### 8.3 I/O Characteristics

**Cache Hit Rates:**
- Root node: 100% (always cached)
- Level 1 internal: >99% (fits in cache)
- Level 2 internal: >95%
- Leaf nodes: ~50% (working set varies)

**Expected I/O per Operation:**
- With cache: 1-2 disk reads on average
- Without cache: 3-4 disk reads (tree height)
- Writes: Batched via dirty tracking

**Total I/O for 300K Operations:**
- Reads: ~300K operations * 1.5 = 450K reads
- With 4KB nodes: ~1.8 GB read
- Writes: ~6K nodes * 4KB = 24 MB written
- Time: ~2-3 seconds (SSD)

---

## 9. Implementation Milestones

### Phase 1: Foundation (Week 1)

**Milestone 1.1: File I/O Layer** (2 days)
- [ ] Implement `FileHeader` struct with serialize/deserialize
- [ ] Implement `FileManager` class
  - [ ] File creation/opening
  - [ ] Header load/save
  - [ ] Node allocation (bump allocator)
  - [ ] Raw node read/write
- [ ] Test: Create file, allocate nodes, read/write, reopen

**Milestone 1.2: Node Structures** (2 days)
- [ ] Implement `InternalNode` struct
  - [ ] Serialize/deserialize
  - [ ] `find_child_index()` with binary search
- [ ] Implement `LeafEntry` and `LeafNode` structs
  - [ ] Serialize/deserialize (variable-length)
  - [ ] `find_entry()` with binary search
  - [ ] `add_value()` / `remove_value()` (sorted)
- [ ] Test: Serialize/deserialize round-trip

**Milestone 1.3: Basic Tree Operations** (3 days)
- [ ] Implement `BPlusTree` class skeleton
- [ ] Implement `find()` operation
  - [ ] `find_leaf()` traversal
  - [ ] Search in leaf node
- [ ] Test: Insert single leaf manually, test find

**Deliverable:** Basic tree that can find entries (no insert yet)

---

### Phase 2: Core Functionality (Week 2)

**Milestone 2.1: Insert Without Splitting** (2 days)
- [ ] Implement `insert()` for non-full nodes
  - [ ] Find leaf
  - [ ] Insert value into existing entry
  - [ ] Create new entry if key doesn't exist
- [ ] Test: Insert into single leaf, verify persistence

**Milestone 2.2: Node Splitting** (3 days)
- [ ] Implement `split_leaf()`
  - [ ] Create new leaf node
  - [ ] Redistribute entries
  - [ ] Update next_leaf pointers
- [ ] Implement `insert_into_parent()`
  - [ ] Insert key into parent
  - [ ] Create new root if needed
- [ ] Implement `split_internal()`
  - [ ] Create new internal node
  - [ ] Redistribute keys and children
- [ ] Test: Insert until split occurs

**Milestone 2.3: Delete Operation** (2 days)
- [ ] Implement `delete()` (simplified)
  - [ ] Find leaf
  - [ ] Remove value from entry
  - [ ] Remove entry if no values left
  - [ ] No merging (nodes can be sparse)
- [ ] Test: Delete existing and non-existing entries

**Deliverable:** Fully functional B+ Tree (insert/find/delete)

---

### Phase 3: Optimization (Week 3)

**Milestone 3.1: Node Cache** (2 days)
- [ ] Implement `NodeCache` class
  - [ ] LRU eviction policy
  - [ ] Dirty tracking
  - [ ] Flush on eviction/close
- [ ] Integrate cache into `BPlusTree`
- [ ] Test: Verify cache hits, measure performance

**Milestone 3.2: Performance Tuning** (2 days)
- [ ] Profile with large datasets
- [ ] Optimize hot paths (binary search, serialization)
- [ ] Tune cache size
- [ ] Add buffered I/O to FileManager
- [ ] Test: 300K operations in <5 seconds

**Milestone 3.3: Robustness** (3 days)
- [ ] Handle edge cases
  - [ ] Empty tree
  - [ ] Single entry
  - [ ] Duplicate values
  - [ ] Delete non-existent
- [ ] Add assertions and error checking
- [ ] Test persistence across program runs
- [ ] Test: Sample input from problem

**Deliverable:** Production-ready implementation

---

### Phase 4: Testing & Submission (Week 4)

**Milestone 4.1: Comprehensive Testing** (3 days)
- [ ] Unit tests for each component
- [ ] Integration tests for operations
- [ ] Stress tests (300K operations)
- [ ] Edge case tests
- [ ] Test: All tests pass

**Milestone 4.2: Build System** (1 day)
- [ ] Create `CMakeLists.txt`
- [ ] Create `Makefile`
- [ ] Create `.gitignore`
- [ ] Test: Clean build produces `code` executable

**Milestone 4.3: Documentation** (1 day)
- [ ] Code comments
- [ ] README with usage instructions
- [ ] Performance notes
- [ ] Known limitations

**Milestone 4.4: Submission** (2 days)
- [ ] Final code review
- [ ] Run sample input
- [ ] Commit to repository
- [ ] Submit to ACMOJ
- [ ] Monitor results

**Deliverable:** Submitted solution, passing all tests

---

## 10. Testing Strategy

### 10.1 Unit Tests

**File I/O:**
- File creation/reopening
- Header persistence
- Node allocation sequence
- Read/write correctness

**Serialization:**
- Internal node round-trip
- Leaf node round-trip (various value counts)
- Empty nodes
- Full nodes

**Node Operations:**
- Binary search in internal nodes
- Binary search in leaf nodes
- Value insertion (sorted order)
- Value removal

### 10.2 Integration Tests

**Basic Operations:**
```
Test: Insert and find single entry
  insert key1 100
  find key1 → 100

Test: Multiple values for same key
  insert key1 100
  insert key1 50
  insert key1 150
  find key1 → 50 100 150

Test: Delete value
  insert key1 100
  insert key1 200
  delete key1 100
  find key1 → 200

Test: Delete non-existent
  delete key1 999
  (no error, no-op)

Test: Empty result
  find nonexistent → null
```

**Complex Scenarios:**
```
Test: Node splitting
  Insert 60 entries (force split)
  Verify all findable
  Verify sorted order

Test: Multiple levels
  Insert 3000+ entries (force 3-level tree)
  Verify all findable

Test: Persistence
  Run 1: Insert entries, exit
  Run 2: Find entries (should exist)
```

### 10.3 Performance Tests

**Large Dataset:**
```
Test: 300K mixed operations
  100K inserts
  100K finds
  100K deletes
  Time: <5 seconds
  Memory: <64 MiB
```

**Cache Effectiveness:**
```
Test: Measure cache hit rate
  Expected: >90% for mixed workload
```

### 10.4 Edge Cases

- Empty tree find → null
- Insert into empty tree
- Delete last value from entry
- Delete last entry from leaf
- Keys with special characters
- Maximum key length (64 bytes)
- Negative values
- INT_MIN, INT_MAX values
- Identical keys with different values

---

## 11. Risk Analysis

### 11.1 Technical Risks

| Risk                          | Likelihood | Impact | Mitigation                                    |
|-------------------------------|------------|--------|-----------------------------------------------|
| Node overflow (>4096 bytes)   | Medium     | High   | Conservative MAX_ENTRIES limit, validation    |
| Cache thrashing               | Low        | Medium | Tune cache size, ensure root always cached    |
| File corruption               | Low        | High   | Magic number check, version validation        |
| Memory leak                   | Low        | Medium | RAII, smart pointers, thorough testing        |
| Performance regression        | Medium     | Medium | Profile regularly, performance tests in CI    |
| Integer overflow              | Low        | Low    | Use size_t for counts, validate inputs        |

### 11.2 Implementation Risks

| Risk                          | Likelihood | Impact | Mitigation                                    |
|-------------------------------|------------|--------|-----------------------------------------------|
| Binary search bugs            | Medium     | High   | Thorough testing, reference implementation    |
| Serialization endianness      | Low        | Medium | Use fixed endianness (system native OK)       |
| Parent pointer maintenance    | Medium     | Medium | Simplify: don't track parent pointers         |
| Split logic errors            | High       | High   | Incremental testing, visual debugging         |
| Delete without merge issues   | Low        | Low    | Simplified delete reduces complexity          |

### 11.3 Schedule Risks

| Risk                          | Likelihood | Impact | Mitigation                                    |
|-------------------------------|------------|--------|-----------------------------------------------|
| Underestimated complexity     | Medium     | Medium | Phase 3 has buffer time for overruns          |
| Bug hunting takes too long    | Medium     | Medium | Invest in early testing, assertions           |
| Performance issues late       | Low        | High   | Performance testing in Phase 3, not Phase 4   |
| ACMOJ submission failures     | Low        | Low    | Test sample locally first, incremental submit |

### 11.4 Acceptance Criteria Risks

| Risk                          | Likelihood | Impact | Mitigation                                    |
|-------------------------------|------------|--------|-----------------------------------------------|
| Time limit exceeded           | Low        | High   | Cache optimization, profile hot paths         |
| Memory limit exceeded         | Very Low   | High   | Fixed-size cache, no unbounded growth         |
| Wrong output format           | Low        | Medium | Match sample exactly, thorough output testing |
| Hidden test case failure      | Medium     | Medium | Comprehensive edge case testing               |

---

## Appendix A: Key Design Choices

### Why B+ Tree over B-Tree?
- Leaf nodes linked for sequential access (useful for debugging)
- All data in leaves (simpler internal node structure)
- Better cache locality for range queries

### Why 4KB Node Size?
- Aligns with typical disk page size
- Good balance: 56-way fanout (internal), ~50 entries (leaf)
- Minimizes tree height (3-4 levels for 300K entries)

### Why Single File?
- Simpler file management
- Better locality (sequential allocation)
- Fewer file handles
- Easier to clean up

### Why No Node Merging?
- Reduces complexity significantly
- Sparse nodes acceptable for this problem
- Deletion is rare in test cases
- Performance impact minimal

### Why LRU Cache?
- Root and upper levels naturally stay hot
- Simple eviction policy
- Bounded memory usage
- High hit rate (>90%)

### Why Fixed-Size Nodes?
- Simplifies allocation (bump allocator)
- Predictable memory usage
- Fast offset calculation
- Easy serialization

---

## Appendix B: References

**Background Research:**
- Sophia's research report: `workspace/sophia/research_report.md`
- Implementation guide: `workspace/sophia/implementation_guide.md`
- Visual guide: `workspace/sophia/VISUAL_GUIDE.md`

**Academic Sources:**
- "The Ubiquitous B-Tree" by Comer (1979)
- "Modern B-Tree Techniques" by Goetz Graefe (2011)

**Implementation Guides:**
- SQLite B-Tree implementation
- BerkeleyDB B+Tree design

---

**End of Architecture Document**

*This architecture is designed to be implementable in 3-4 weeks by a small team, with clear milestones and testable deliverables at each phase. The design prioritizes simplicity, correctness, and performance within the stated resource constraints.*

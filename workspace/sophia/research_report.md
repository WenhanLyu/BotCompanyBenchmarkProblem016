# B+ Tree File Storage Research Report

## Executive Summary

This report provides comprehensive research on implementing a file-based B+ Tree for a key-value database. The implementation must handle up to 300,000 operations with strict resource constraints: 64 MiB memory, 1024 MiB disk space, and maximum 20 files.

**Key Findings:**
- B+ Trees are optimal for file-based storage due to sequential leaf access and high fanout
- Recommended node size: 4KB (disk page size alignment)
- Multi-value support required (same key, different values)
- Critical optimizations: LRU cache, lazy writing, and minimal tree height

---

## 1. B+ Tree Fundamentals

### 1.1 Why B+ Trees for File Storage?

B+ Trees are specifically designed for disk-based storage systems and offer several advantages over regular B-Trees:

1. **All data in leaf nodes**: Internal nodes only store keys for navigation, maximizing fanout
2. **Sequential access**: Leaf nodes are linked, enabling efficient range queries
3. **Consistent performance**: All searches traverse the same depth (balanced tree)
4. **High fanout**: Minimizes tree height, reducing disk I/O operations
5. **Efficient range queries**: Linked leaves allow iteration without tree traversal

### 1.2 Structure Components

#### Internal Nodes
```
Structure:
[P0][K1][P1][K2][P2]...[Kn][Pn]

- Ki: Keys (search values)
- Pi: Pointers to child nodes
- Property: K(i-1) ≤ keys in subtree Pi < Ki
- Fanout: Typically (node_size / (key_size + pointer_size))
```

For our problem:
- Key: 64-byte string
- Pointer: 8 bytes (file offset)
- Node size: 4096 bytes (recommended)
- Internal node fanout: ~55 children per node

#### Leaf Nodes
```
Structure:
[K1][V1,1][V1,2]...[K2][V2,1]...[Kn][Vn,1]...[Next]

- Ki: Keys (index strings)
- Vi,j: Multiple values for same key (sorted)
- Next: Pointer to next leaf node (linked list)
- Property: Keys in ascending order
```

For our problem:
- Must support multiple values per key
- Values must be stored in sorted order
- Need to handle up to 300,000 entries

### 1.3 Key Properties

1. **Order (M)**: Maximum number of children per internal node
2. **Minimum fill**: Internal nodes must be at least ⌈M/2⌉ full (except root)
3. **Balanced**: All leaf nodes at the same level
4. **Sorted**: Keys always in ascending order
5. **Linked leaves**: Enables efficient range scans

---

## 2. Core Algorithms

### 2.1 Search Algorithm

**Time Complexity**: O(log_M N) where M is fanout, N is total entries

```
Pseudocode:
function FIND(key):
    node = load_node(root_offset)
    
    # Traverse to leaf
    while node is internal:
        i = binary_search(node.keys, key)
        node = load_node(node.pointers[i])
    
    # Search in leaf
    values = []
    for entry in node.entries:
        if entry.key == key:
            values.extend(entry.values)
        elif entry.key > key:
            break
    
    return sorted(values) if values else null
```

**File I/O**: At most height + 1 disk reads (can be optimized with caching)

### 2.2 Insertion Algorithm

**Time Complexity**: O(log_M N) + split overhead

```
Pseudocode:
function INSERT(key, value):
    # Find insertion point
    path = []
    node = load_node(root_offset)
    
    while node is internal:
        path.append((node, index))
        i = find_child_index(node, key)
        node = load_node(node.pointers[i])
    
    # Insert into leaf
    if key exists in leaf:
        if value not in leaf.values[key]:
            leaf.values[key].append(value)
            leaf.values[key].sort()
    else:
        leaf.add_entry(key, [value])
    
    # Handle overflow
    if leaf.size > MAX_LEAF_SIZE:
        split_leaf(leaf, path)
    
    save_node(leaf)

function SPLIT_LEAF(leaf, path):
    # Create new leaf
    mid = len(leaf.entries) / 2
    new_leaf = create_node(LEAF)
    new_leaf.entries = leaf.entries[mid:]
    leaf.entries = leaf.entries[:mid]
    
    # Link leaves
    new_leaf.next = leaf.next
    leaf.next = new_leaf.offset
    
    # Promote key to parent
    promote_key = new_leaf.entries[0].key
    insert_into_parent(path, promote_key, new_leaf.offset)

function INSERT_INTO_PARENT(path, key, new_child_offset):
    if path is empty:
        # Create new root
        new_root = create_internal_node()
        new_root.keys = [key]
        new_root.pointers = [old_root_offset, new_child_offset]
        root_offset = new_root.offset
    else:
        parent = path.pop()
        parent.insert(key, new_child_offset)
        if parent.size > MAX_INTERNAL_SIZE:
            split_internal(parent, path)
```

**Critical Points:**
1. Must maintain sorted order of values for same key
2. Splitting propagates up the tree (may increase height)
3. Each split requires writing 2 nodes + updating parent

### 2.3 Deletion Algorithm

**Time Complexity**: O(log_M N) + merge/redistribute overhead

```
Pseudocode:
function DELETE(key, value):
    # Find target leaf
    path = []
    node = load_node(root_offset)
    
    while node is internal:
        path.append(node)
        i = find_child_index(node, key)
        node = load_node(node.pointers[i])
    
    # Delete from leaf
    if key exists in leaf:
        if value in leaf.values[key]:
            leaf.values[key].remove(value)
            if leaf.values[key] is empty:
                leaf.remove_entry(key)
        else:
            return  # Entry doesn't exist, no-op
    else:
        return  # Key doesn't exist, no-op
    
    # Handle underflow
    if leaf.size < MIN_LEAF_SIZE and leaf != root:
        merge_or_redistribute(leaf, path)
    
    save_node(leaf)

function MERGE_OR_REDISTRIBUTE(leaf, path):
    parent = path[-1]
    sibling = find_sibling(leaf, parent)
    
    if leaf.size + sibling.size <= MAX_LEAF_SIZE:
        # Merge
        merge_leaves(leaf, sibling)
        remove_from_parent(parent, key_between)
    else:
        # Redistribute
        redistribute_entries(leaf, sibling)
        update_parent_key(parent, new_separator)
```

**Critical Points:**
1. Deletion may not exist (no-op, not an error)
2. Underflow handling is complex but optional for performance
3. Can defer merging to maintain simpler code

---

## 3. File-Based Storage Strategies

### 3.1 File Layout Options

#### Option A: Single File with Fixed-Size Nodes
```
File Structure:
[Header][Node 0][Node 1][Node 2]...[Node N]

Header (256 bytes):
- Magic number (validation)
- Root node offset
- Next free offset
- Node count
- Tree metadata (order, height)

Node (4096 bytes):
- Node type (1 byte): INTERNAL or LEAF
- Entry count (4 bytes)
- Parent offset (8 bytes) - optional, helps navigation
- Payload (variable)
```

**Pros:**
- Simple offset calculation: offset = HEADER_SIZE + node_id * NODE_SIZE
- Easy to manage free space
- Single file = simpler I/O

**Cons:**
- Fixed node size may waste space
- Fragmentation over time

#### Option B: Multiple Files (Separate Index and Data)
```
index.bpt: B+ Tree structure (keys and pointers)
data.bpt: Actual values
meta.bpt: Metadata
```

**Pros:**
- Can optimize each file separately
- Smaller index file improves cache hit rate

**Cons:**
- More complex management
- Higher file descriptor usage (limited to 20 files)

**Recommendation**: Use Option A (single file) for simplicity and efficiency.

### 3.2 Node Serialization Format

#### Internal Node Format (4096 bytes)
```
Offset  | Size | Field
--------|------|------------------
0       | 1    | Node type (0x01)
1       | 4    | Entry count (n)
5       | 8    | Parent offset
13      | 3    | Padding
16      | 72   | Entry 0: [64-byte key][8-byte pointer]
88      | 72   | Entry 1
...     | ...  | ...
```

Max entries: (4096 - 16) / 72 ≈ 56 entries

#### Leaf Node Format (4096 bytes)
```
Offset  | Size | Field
--------|------|------------------
0       | 1    | Node type (0x02)
1       | 4    | Entry count (n)
5       | 8    | Next leaf offset
13      | 3    | Padding
16      | var  | Entry 0: [64-byte key][4-byte value count][values...]
...     | ...  | ...
```

**Variable Entry Size Challenge:**
- Each key can have multiple values
- Values must be sorted
- Need efficient packing

**Solution**: Pre-allocate max values per key or use offset table within node.

### 3.3 Free Space Management

#### Strategy 1: Append-Only with Tombstones
```
- New nodes always appended to end
- Deleted nodes marked as free (tombstone)
- Periodic compaction to reclaim space
```

**Pros**: Simple, no fragmentation during operation
**Cons**: File grows continuously, requires compaction

#### Strategy 2: Free List
```
Header contains:
- free_list_head: offset to first free node

Each free node contains:
- next_free: offset to next free node
```

**Pros**: Efficient space reuse
**Cons**: More complex allocation/deallocation

**Recommendation**: Start with append-only, add free list if disk space becomes issue.

### 3.4 Persistence and Crash Recovery

**Key Requirements:**
1. **Atomic writes**: Ensure node updates are atomic
2. **Consistent state**: Tree remains valid after crash
3. **Idempotent operations**: Operations can be repeated safely

**Simple Strategy:**
```
1. Write new/modified nodes to disk
2. Update parent pointers
3. Update root offset in header (atomic operation)
```

**Advanced Strategy (Write-Ahead Logging):**
```
1. Write operation to log
2. Perform operation
3. Mark log entry as complete
4. Checkpoint periodically
```

For this problem, simple strategy is sufficient since judge handles cleanup between tests.

---

## 4. Performance Optimization Strategies

### 4.1 Memory Management (64 MiB Constraint)

**Cache Strategy: LRU (Least Recently Used)**

```cpp
class NodeCache {
    static const size_t MAX_CACHE_SIZE = 1000; // ~4MB for 4KB nodes
    
    unordered_map<offset_t, Node*> cache;
    list<offset_t> lru_list;
    unordered_map<offset_t, list<offset_t>::iterator> lru_map;
    
    Node* get(offset_t offset) {
        if (cache.contains(offset)) {
            // Move to front (most recently used)
            lru_list.erase(lru_map[offset]);
            lru_list.push_front(offset);
            lru_map[offset] = lru_list.begin();
            return cache[offset];
        }
        
        // Load from disk
        Node* node = load_from_disk(offset);
        
        // Evict if necessary
        if (cache.size() >= MAX_CACHE_SIZE) {
            offset_t evict = lru_list.back();
            if (cache[evict]->is_dirty()) {
                write_to_disk(cache[evict]);
            }
            delete cache[evict];
            cache.erase(evict);
            lru_map.erase(evict);
            lru_list.pop_back();
        }
        
        // Add to cache
        cache[offset] = node;
        lru_list.push_front(offset);
        lru_map[offset] = lru_list.begin();
        
        return node;
    }
};
```

**Memory Budget Breakdown:**
- Node cache: ~4 MB (1000 nodes × 4KB)
- Working memory: ~8 MB (path stack, temporary structures)
- STL containers: ~40 MB (std::map for in-memory operations)
- Buffer: ~12 MB (reserve)

### 4.2 Disk I/O Optimization

**Key Principles:**
1. **Minimize reads**: Cache frequently accessed nodes (root, high-level internal nodes)
2. **Batch writes**: Group node writes together
3. **Sequential access**: Leverage disk sequential read speed
4. **Alignment**: Align nodes to 4KB boundaries (disk page size)

**Techniques:**

#### 1. Pin hot nodes in cache
```cpp
// Root and top-level nodes should stay in cache
cache.pin(root_offset);
for (auto child_offset : root.pointers) {
    cache.pin(child_offset);
}
```

#### 2. Lazy writing
```cpp
// Mark nodes as dirty, write at end of operation
node->mark_dirty();

// Flush all dirty nodes before program exit
void flush_all() {
    for (auto [offset, node] : cache) {
        if (node->is_dirty()) {
            write_to_disk(node);
        }
    }
}
```

#### 3. Buffered I/O
```cpp
// Use larger buffer for file operations
FILE* fp = fopen("data.bpt", "rb+");
setvbuf(fp, buffer, _IOFBF, 64 * 1024); // 64KB buffer
```

### 4.3 Node Size Selection

**Trade-off Analysis:**

| Node Size | Fanout | Height (300K) | I/O per Op | Wasted Space |
|-----------|--------|---------------|------------|--------------|
| 512 B     | ~7     | ~6            | ~7 reads   | Low          |
| 1024 B    | ~14    | ~5            | ~6 reads   | Low          |
| 4096 B    | ~56    | ~3            | ~4 reads   | Medium       |
| 8192 B    | ~113   | ~2            | ~3 reads   | High         |

**Calculation for 4KB nodes:**
- Fanout ≈ 56
- Height = log₅₆(300,000) ≈ 3.2 → 4 levels
- I/O per search = 4 disk reads (without cache)
- With cache (root + level 1): 2 disk reads

**Recommendation: 4096 bytes (4KB)**
- Aligns with disk page size (standard)
- Good balance between fanout and space efficiency
- Height of 3-4 for 300K entries
- Manageable memory footprint

### 4.4 Search Optimization

**Binary Search within Nodes:**
```cpp
// For internal node with n keys
int find_child_index(InternalNode* node, const string& key) {
    // Binary search in sorted key array
    int left = 0, right = node->count;
    while (left < right) {
        int mid = (left + right) / 2;
        if (key < node->keys[mid]) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    return left;
}

// For leaf node
vector<int> find_values(LeafNode* node, const string& key) {
    // Binary search to find key
    int idx = binary_search_key(node, key);
    if (idx >= 0) {
        return node->values[idx]; // Already sorted
    }
    return {};
}
```

**Time Complexity:**
- Within node: O(log n) where n is entries per node
- Tree traversal: O(log_M N) where M is fanout, N is total entries
- Overall: O(log n × log_M N) ≈ O(log N) with good constant factor

### 4.5 Insertion Optimization

**Strategies:**
1. **Lazy splitting**: Split only when absolutely necessary
2. **Bulk loading**: If inserting many keys at once, build tree bottom-up
3. **Pre-allocate space**: For values array within leaf node

**Avoiding Unnecessary Splits:**
```cpp
// Only split if node is completely full
if (leaf->count >= MAX_ENTRIES) {
    split(leaf);
}

// Consider redistribution before splitting
if (leaf->count >= MAX_ENTRIES) {
    if (can_redistribute_to_sibling(leaf)) {
        redistribute(leaf);
    } else {
        split(leaf);
    }
}
```

### 4.6 Deletion Optimization

**Simplified Approach:**
```cpp
// No merging or redistribution
// Just remove entry, even if node becomes sparse
// This is acceptable for the problem constraints

void delete_entry(const string& key, int value) {
    LeafNode* leaf = find_leaf(key);
    auto& values = leaf->get_values(key);
    
    auto it = std::find(values.begin(), values.end(), value);
    if (it != values.end()) {
        values.erase(it);
        if (values.empty()) {
            leaf->remove_key(key);
        }
        leaf->mark_dirty();
    }
}
```

**Pros:**
- Much simpler implementation
- Fewer disk writes
- No risk of cascade merges

**Cons:**
- Tree may become sparse over time
- Slight performance degradation with many deletions

**Verdict**: For 300K operations with tight deadline, simplified deletion is acceptable.

---

## 5. Multi-Value Handling

### 5.1 Problem Requirements

From the problem statement:
- Same index can have multiple values
- Same index cannot have duplicate values
- Values must be output in ascending order
- Delete only removes specific (index, value) pair

### 5.2 Data Structure

**Leaf Node Entry:**
```cpp
struct LeafEntry {
    char key[64];           // Index string
    int value_count;        // Number of values
    int values[MAX_VALUES]; // Sorted array of values
};
```

**Alternative (more flexible):**
```cpp
struct LeafEntry {
    char key[64];
    vector<int> values; // Automatically sorted on insert
};
```

### 5.3 Operations

**Insert with duplicate key:**
```cpp
void insert(const string& key, int value) {
    LeafEntry* entry = find_entry(key);
    if (entry) {
        // Key exists, add value if not duplicate
        auto it = std::lower_bound(entry->values.begin(), 
                                   entry->values.end(), 
                                   value);
        if (it == entry->values.end() || *it != value) {
            entry->values.insert(it, value);
        }
    } else {
        // New key
        create_entry(key, {value});
    }
}
```

**Find (return all values):**
```cpp
vector<int> find(const string& key) {
    LeafEntry* entry = find_entry(key);
    if (entry) {
        return entry->values; // Already sorted
    }
    return {};
}
```

**Delete specific value:**
```cpp
void delete_entry(const string& key, int value) {
    LeafEntry* entry = find_entry(key);
    if (entry) {
        auto it = std::find(entry->values.begin(), 
                           entry->values.end(), 
                           value);
        if (it != entry->values.end()) {
            entry->values.erase(it);
            if (entry->values.empty()) {
                remove_entry(key);
            }
        }
    }
    // If not found, no-op (per problem requirement)
}
```

---

## 6. Implementation Roadmap

### 6.1 Phase 1: Core Structure
1. Define node structures (internal and leaf)
2. Implement file I/O primitives (read/write node)
3. Create basic tree initialization

### 6.2 Phase 2: Basic Operations
1. Implement search/find
2. Implement insert (without splitting)
3. Implement delete (without merging)
4. Test with small datasets

### 6.3 Phase 3: Tree Maintenance
1. Implement node splitting
2. Implement parent updates
3. Handle root splitting (height increase)
4. Test with larger datasets

### 6.4 Phase 4: Optimization
1. Add LRU cache
2. Implement lazy writing
3. Optimize node layout
4. Profile and tune

### 6.5 Phase 5: Edge Cases
1. Handle empty tree
2. Handle single-node tree
3. Handle very long value lists
4. Persistence across test cases

---

## 7. Potential Pitfalls and Edge Cases

### 7.1 Common Bugs

**1. Off-by-one errors in node splitting**
```cpp
// WRONG: May create invalid tree
mid = node->count / 2;
new_node->entries = node->entries[mid:];  // Should promote mid key

// CORRECT:
mid = node->count / 2;
promote_key = node->entries[mid].key;
new_node->entries = node->entries[mid+1:];
```

**2. Forgetting to update parent pointers**
```cpp
// After split, must update parent
parent->insert_key(promote_key, new_node_offset);
```

**3. Not maintaining sorted order**
```cpp
// Must keep values sorted
values.push_back(new_value);
std::sort(values.begin(), values.end()); // Don't forget!

// Better: use binary search insert
auto it = std::lower_bound(values.begin(), values.end(), new_value);
values.insert(it, new_value);
```

**4. File pointer management**
```cpp
// Always seek before read/write
fseek(fp, offset, SEEK_SET);
fread(buffer, node_size, 1, fp);

// Or use pread/pwrite for thread safety
pread(fd, buffer, node_size, offset);
```

**5. Buffer overflow with strings**
```cpp
// Keys are max 64 bytes, ensure null termination
char key[65]; // Extra byte for '\0'
strncpy(key, input, 64);
key[64] = '\0';
```

### 7.2 Edge Cases

**1. Empty tree**
```cpp
// Find on empty tree should return null
if (root_offset == INVALID_OFFSET) {
    return {};
}
```

**2. Deleting from single-node tree**
```cpp
// Don't try to merge/redistribute
if (node == root && node->is_leaf()) {
    // Just remove entry, don't worry about underflow
    node->remove(key, value);
}
```

**3. Very long value lists**
```cpp
// A single key might have many values
// Ensure leaf node can accommodate or handle overflow
if (entry->values.size() > MAX_VALUES_PER_ENTRY) {
    // Option 1: Split entry across multiple leaves
    // Option 2: Use overflow page
    // Option 3: Increase node size
}
```

**4. File persistence**
```cpp
// Must flush before program exit
void cleanup() {
    cache.flush_all();
    fclose(fp);
}

// Register cleanup handler
atexit(cleanup);
```

**5. Test continuation**
```cpp
// Problem states: "Some test cases require continuing 
// operations based on previous run results"

// On startup:
if (file_exists("data.bpt")) {
    // Load existing tree
    load_tree();
} else {
    // Create new tree
    initialize_tree();
}
```

### 7.3 Performance Edge Cases

**1. Highly skewed insertions**
```cpp
// All inserts in ascending order
// B+ Tree handles this well (sequential inserts)
// But consider optimizing for this case
```

**2. Many deletions**
```cpp
// Tree becomes sparse, performance degrades
// For this problem, acceptable (no rebalancing required)
```

**3. Cache thrashing**
```cpp
// Random access pattern may cause poor cache performance
// Ensure cache size is adequate (at least root + level 1)
```

---

## 8. Reference Implementation Patterns

### 8.1 File Structure Pattern

```cpp
class BPlusTree {
private:
    FILE* fp;
    offset_t root_offset;
    NodeCache cache;
    
    static const size_t NODE_SIZE = 4096;
    static const size_t HEADER_SIZE = 256;
    
public:
    BPlusTree(const char* filename) {
        if (file_exists(filename)) {
            fp = fopen(filename, "rb+");
            load_header();
        } else {
            fp = fopen(filename, "wb+");
            initialize_header();
        }
    }
    
    ~BPlusTree() {
        cache.flush_all();
        save_header();
        fclose(fp);
    }
    
    void insert(const string& key, int value);
    void delete_entry(const string& key, int value);
    vector<int> find(const string& key);
};
```

### 8.2 Node Class Pattern

```cpp
class Node {
protected:
    offset_t offset;
    bool dirty;
    
public:
    virtual void serialize(char* buffer) = 0;
    virtual void deserialize(const char* buffer) = 0;
    virtual bool is_leaf() = 0;
    
    void mark_dirty() { dirty = true; }
    bool is_dirty() const { return dirty; }
};

class InternalNode : public Node {
    int count;
    char keys[MAX_KEYS][64];
    offset_t pointers[MAX_KEYS + 1];
    
public:
    bool is_leaf() override { return false; }
    void serialize(char* buffer) override;
    void deserialize(const char* buffer) override;
};

class LeafNode : public Node {
    int count;
    struct Entry {
        char key[64];
        vector<int> values;
    } entries[MAX_ENTRIES];
    offset_t next;
    
public:
    bool is_leaf() override { return true; }
    void serialize(char* buffer) override;
    void deserialize(const char* buffer) override;
};
```

### 8.3 Operation Pattern

```cpp
vector<int> BPlusTree::find(const string& key) {
    if (root_offset == INVALID_OFFSET) {
        return {};
    }
    
    // Traverse to leaf
    Node* node = cache.get(root_offset);
    while (!node->is_leaf()) {
        InternalNode* internal = static_cast<InternalNode*>(node);
        int idx = internal->find_child_index(key);
        node = cache.get(internal->pointers[idx]);
    }
    
    // Search in leaf
    LeafNode* leaf = static_cast<LeafNode*>(node);
    return leaf->find_values(key);
}

void BPlusTree::insert(const string& key, int value) {
    if (root_offset == INVALID_OFFSET) {
        create_initial_root();
    }
    
    // Find leaf and insert
    LeafNode* leaf = find_leaf(key);
    bool overflow = leaf->insert(key, value);
    
    if (overflow) {
        split_leaf(leaf);
    }
}
```

---

## 9. Testing Strategy

### 9.1 Unit Tests

1. **Node operations**
   - Serialize/deserialize
   - Insert/delete within node
   - Split node

2. **Tree operations**
   - Insert into empty tree
   - Insert duplicate keys
   - Find existing/non-existing keys
   - Delete existing/non-existing entries

3. **File persistence**
   - Save and reload tree
   - Verify integrity after reload

### 9.2 Integration Tests

1. **Sample input** (from problem)
2. **Stress test** (300K operations)
3. **Random operations** (mixed insert/delete/find)
4. **Edge cases** (see section 7.2)

### 9.3 Performance Tests

1. **Time measurement** (must be under 5 seconds)
2. **Memory usage** (must be under 64 MiB)
3. **Disk usage** (must be under 1024 MiB)
4. **File count** (must be ≤ 20)

---

## 10. Key Takeaways for Implementation Team

### 10.1 Critical Design Decisions

1. **Node size: 4096 bytes** (page-aligned, good fanout)
2. **Single file storage** (simple, efficient)
3. **LRU cache with ~1000 nodes** (fits in memory budget)
4. **Lazy writing** (reduce disk I/O)
5. **No merge on delete** (simplify implementation)

### 10.2 Must-Have Features

1. ✅ Multi-value support per key
2. ✅ Sorted value output
3. ✅ Handle non-existent deletes gracefully
4. ✅ File persistence across test cases
5. ✅ Efficient search (O(log N))

### 10.3 Nice-to-Have Optimizations

1. 🔄 Node redistribution (instead of immediate split)
2. 🔄 Bulk loading optimization
3. 🔄 Compression for sparse nodes
4. 🔄 Write-ahead logging for crash recovery

### 10.4 Implementation Priority

**Week 1:**
- Basic structure and file I/O ✓
- Simple insert/find/delete ✓
- Pass sample test ✓

**Week 2:**
- Node splitting ✓
- Tree balancing ✓
- Basic caching ✓

**Week 3:**
- Performance optimization ✓
- Edge case handling ✓
- Full test suite ✓

---

## 11. Recommended Resources

### 11.1 Academic References

1. **"Introduction to Algorithms" (CLRS)** - Chapter 18: B-Trees
2. **"Database System Concepts" (Silberschatz)** - Chapter 11: Indexing
3. **Original B-Tree paper**: Bayer & McCreight (1972)

### 11.2 Implementation Examples

1. **SQLite B-Tree**: Simple, well-documented implementation
2. **Berkeley DB**: Production-grade B+ Tree
3. **LevelDB**: LSM-tree (alternative approach, but has good file I/O patterns)

### 11.3 Online Resources

1. **Visualization**: https://www.cs.usfca.edu/~galles/visualization/BPlusTree.html
2. **Tutorial**: GeeksforGeeks B+ Tree articles
3. **Lecture notes**: CMU 15-445 Database Systems

---

## 12. Conclusion

B+ Trees are the ideal data structure for this file-based key-value database due to:
- **Excellent disk I/O characteristics** (high fanout, balanced depth)
- **Support for range queries** (though not required here)
- **Predictable performance** (all operations O(log N))
- **Well-understood implementation** (mature algorithms)

The recommended implementation approach:
1. **4KB fixed-size nodes** in a single file
2. **LRU cache** for frequently accessed nodes
3. **Lazy writing** to minimize disk I/O
4. **Simplified deletion** (no merging) to reduce complexity
5. **Multi-value support** via sorted arrays in leaf nodes

With careful implementation following this research, the system should easily handle 300K operations within the 5-second time limit and 64 MiB memory constraint.

**Estimated performance for 300K operations:**
- Tree height: 3-4 levels
- Cache hit rate: ~95% (for root and high-level nodes)
- Average I/O per operation: 1-2 disk reads
- Total time: ~2-3 seconds (well under 5-second limit)
- Memory usage: ~8-16 MiB (well under 64 MiB limit)
- Disk usage: ~300 MB for 300K entries (well under 1024 MiB limit)

This research provides a solid foundation for the implementation team to build a high-quality, performant B+ Tree file storage system.

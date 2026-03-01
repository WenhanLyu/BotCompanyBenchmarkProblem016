# B+ Tree Visual Guide

## Tree Structure Diagram

```
                        [Root - Internal Node]
                    /          |         |        \
                   /           |         |         \
              [K="CppPrimer"] [K="FlowersForAlgernon"] [K="Java"]
                 /               |                \
                /                |                 \
         [Internal]         [Internal]          [Internal]
            / | \              / | \               / | \
           /  |  \            /  |  \             /  |  \
      [Leaf] [Leaf] [Leaf] [Leaf] [Leaf] [Leaf] [Leaf] [Leaf]
        ↔      ↔      ↔      ↔      ↔      ↔      ↔      ↔
       (Linked list of leaves for sequential access)
```

## Node Layout in File

```
Offset 0 (Header - 256 bytes):
┌─────────────────────────────────────────────┐
│ Magic: 0x42504C54                           │
│ Version: 1                                  │
│ Root Offset: 256                            │
│ Next Free Offset: 12544                     │
│ Node Count: 3                               │
│ Height: 2                                   │
│ Entry Count: 8                              │
│ ... (padding)                               │
└─────────────────────────────────────────────┘

Offset 256 (Node 0 - Root, 4096 bytes):
┌─────────────────────────────────────────────┐
│ Type: INTERNAL (0x01)                       │
│ Key Count: 2                                │
│ Parent: 0                                   │
├─────────────────────────────────────────────┤
│ Key[0]: "CppPrimer" (64 bytes)              │
│ Pointer[0]: 4352 (offset to child)          │
├─────────────────────────────────────────────┤
│ Key[1]: "FlowersForAlgernon" (64 bytes)     │
│ Pointer[1]: 8448 (offset to child)          │
├─────────────────────────────────────────────┤
│ Pointer[2]: 12544 (offset to child)         │
└─────────────────────────────────────────────┘

Offset 4352 (Node 1 - Leaf, 4096 bytes):
┌─────────────────────────────────────────────┐
│ Type: LEAF (0x02)                           │
│ Entry Count: 1                              │
│ Next Leaf: 8448                             │
├─────────────────────────────────────────────┤
│ Entry[0]:                                   │
│   Key: "CppPrimer" (64 bytes)               │
│   Value Count: 2                            │
│   Values: [2001, 2012]                      │
└─────────────────────────────────────────────┘

Offset 8448 (Node 2 - Leaf, 4096 bytes):
┌─────────────────────────────────────────────┐
│ Type: LEAF (0x02)                           │
│ Entry Count: 2                              │
│ Next Leaf: 12544                            │
├─────────────────────────────────────────────┤
│ Entry[0]:                                   │
│   Key: "Dune" (64 bytes)                    │
│   Value Count: 1                            │
│   Values: [2021]                            │
├─────────────────────────────────────────────┤
│ Entry[1]:                                   │
│   Key: "FlowersForAlgernon" (64 bytes)      │
│   Value Count: 1                            │
│   Values: [1966]                            │
└─────────────────────────────────────────────┘

Offset 12544 (Node 3 - Leaf, 4096 bytes):
┌─────────────────────────────────────────────┐
│ Type: LEAF (0x02)                           │
│ Entry Count: 1                              │
│ Next Leaf: 0 (end of list)                  │
├─────────────────────────────────────────────┤
│ Entry[0]:                                   │
│   Key: "Java" (64 bytes)                    │
│   Value Count: 1                            │
│   Values: [2023]                            │
└─────────────────────────────────────────────┘
```

## Operation Flow Diagrams

### Find Operation: `find "CppPrimer"`

```
Step 1: Read root (offset 256)
   Root is INTERNAL
   Binary search keys: ["CppPrimer", "FlowersForAlgernon"]
   "CppPrimer" <= "CppPrimer" → use pointer[0]
   
Step 2: Follow pointer[0] to offset 4352
   Node is LEAF
   Binary search entries for "CppPrimer"
   Found at index 0
   
Step 3: Return values: [2001, 2012]
   (Already sorted)

Total I/O: 2 reads
```

### Insert Operation: `insert "Dune" 2024`

```
Step 1: Find leaf for "Dune" (same as find)
   Navigate to leaf at offset 8448
   
Step 2: Read leaf node
   Entry exists at index 0
   Current values: [2021]
   
Step 3: Add value 2024
   Binary search insert: [2021, 2024]
   Mark node dirty
   
Step 4: Check if split needed
   Entry count = 2 (not full)
   No split required
   
Step 5: Write dirty node to disk
   Serialize and flush to offset 8448

Total I/O: 2 reads, 1 write
```

### Delete Operation: `delete "Dune" 2021`

```
Step 1: Find leaf for "Dune"
   Navigate to leaf at offset 8448
   
Step 2: Read leaf node
   Entry exists at index 0
   Current values: [2021, 2024]
   
Step 3: Remove value 2021
   Remove from vector: [2024]
   Mark node dirty
   
Step 4: Write dirty node to disk
   Serialize and flush to offset 8448

Total I/O: 2 reads, 1 write
```

### Insert with Split: `insert "Book999" 1999` (when leaf is full)

```
Step 1: Find leaf for "Book999"
   Navigate to leaf (let's say at offset 8448)
   
Step 2: Insert into leaf
   Leaf is full (entry_count = MAX_ENTRIES)
   Need to split
   
Step 3: Split leaf
   Create new leaf at offset 16640
   
   Old leaf (8448):              New leaf (16640):
   ┌─────────────────┐           ┌─────────────────┐
   │ Entry[0]: "AAA" │           │ Entry[0]: "MMM" │
   │ Entry[1]: "BBB" │           │ Entry[1]: "NNN" │
   │ ...             │           │ ...             │
   │ Entry[24]: "LLL"│           │ Entry[24]: "ZZZ"│
   │ Next: 16640 ────┼──────────>│ Next: 12544     │
   └─────────────────┘           └─────────────────┘
   
Step 4: Promote key "MMM" to parent
   Insert into parent's keys array
   Update parent's pointers
   
Step 5: If parent is full, split parent (recursive)
   May propagate up to root
   May create new root (height increases)

Total I/O: Multiple reads/writes depending on splits
```

## Cache Behavior

```
LRU Cache (1000 nodes):

┌─────────────────────────────────────────────┐
│ Most Recently Used                          │
│ ┌─────────────┐                             │
│ │ Root (256)  │ ← Always pinned             │
│ ├─────────────┤                             │
│ │ Node 4352   │ ← Recently accessed         │
│ ├─────────────┤                             │
│ │ Node 8448   │                             │
│ ├─────────────┤                             │
│ │ Node 12544  │                             │
│ ├─────────────┤                             │
│ │ ...         │                             │
│ ├─────────────┤                             │
│ │ Node 99999  │ ← Least recently used       │
│ └─────────────┘                             │
│                                             │
│ Cache full → evict Node 99999               │
│ If dirty → write to disk first              │
└─────────────────────────────────────────────┘

Cache Hit Scenario:
   find "CppPrimer"
   → Root in cache: HIT (no disk read)
   → Leaf 4352 in cache: HIT (no disk read)
   Total I/O: 0 reads! (100% cache hit)

Cache Miss Scenario:
   find "UnseenBook"
   → Root in cache: HIT
   → Leaf not in cache: MISS (disk read)
   → Load leaf into cache
   → Evict LRU if full
   Total I/O: 1 read
```

## Memory Layout

```
Process Memory (64 MiB limit):

┌───────────────────────────────────────────┐
│ Code & Static Data: ~2 MiB                │
├───────────────────────────────────────────┤
│ Stack: ~2 MiB                             │
├───────────────────────────────────────────┤
│ Node Cache (1000 × 4KB): ~4 MiB           │
│ ┌─────────────────────────────────────┐   │
│ │ Cached Node Buffers                 │   │
│ │ + LRU metadata                       │   │
│ └─────────────────────────────────────┘   │
├───────────────────────────────────────────┤
│ Working Memory: ~8 MiB                    │
│ - Path stack during traversal             │
│ - Temporary buffers                       │
│ - String operations                       │
├───────────────────────────────────────────┤
│ STL Containers: ~40 MiB                   │
│ - unordered_map for cache                 │
│ - vectors for values                      │
│ - lists for LRU                           │
├───────────────────────────────────────────┤
│ I/O Buffers: ~8 MiB                       │
│ - FILE* buffer (64 KB)                    │
│ - Temporary read/write buffers           │
└───────────────────────────────────────────┘
Total: ~64 MiB (at limit)
```

## Performance Characteristics

### Tree Height vs Entry Count

```
Fanout = 56 (internal nodes)

N entries    | Height | Max I/O (no cache)
-------------|--------|-------------------
100          | 2      | 3 reads
1,000        | 2      | 3 reads
10,000       | 3      | 4 reads
100,000      | 3      | 4 reads
300,000      | 4      | 5 reads
1,000,000    | 4      | 5 reads

With cache (root + level 1 pinned):
300,000      | 4      | 2-3 reads (typical)
```

### Operation Time Complexity

```
Operation     | Time          | I/O
--------------|---------------|------------------
Find          | O(log N)      | O(h) = O(log N)
Insert        | O(log N)      | O(h) = O(log N)
Delete        | O(log N)      | O(h) = O(log N)
Split         | O(M)          | O(h) (worst case)

Where:
  N = total entries
  h = tree height
  M = node capacity
```

## Example Walkthrough: Sample Input

```
Input:
8
insert FlowersForAlgernon 1966
insert CppPrimer 2012
insert Dune 2021
insert CppPrimer 2001
find CppPrimer
find Java
delete Dune 2021
find Dune

Tree Evolution:

After insert FlowersForAlgernon 1966:
[Leaf: FlowersForAlgernon→[1966]]

After insert CppPrimer 2012:
[Leaf: CppPrimer→[2012], FlowersForAlgernon→[1966]]

After insert Dune 2021:
[Leaf: CppPrimer→[2012], Dune→[2021], FlowersForAlgernon→[1966]]

After insert CppPrimer 2001:
[Leaf: CppPrimer→[2001,2012], Dune→[2021], FlowersForAlgernon→[1966]]
                    ↑ sorted!

find CppPrimer:
Output: 2001 2012

find Java:
Not found
Output: null

After delete Dune 2021:
[Leaf: CppPrimer→[2001,2012], Dune→[], FlowersForAlgernon→[1966]]
                              remove entire entry ↓
[Leaf: CppPrimer→[2001,2012], FlowersForAlgernon→[1966]]

find Dune:
Not found
Output: null
```

## File Growth Pattern

```
Operation Count | Nodes | File Size | Disk Usage
----------------|-------|-----------|------------
0 (empty)       | 0     | 256 B     | 1 page
1-50            | 1     | 4.25 KB   | 2 pages
51-2,500        | ~50   | ~200 KB   | 50 pages
2,501-100,000   | ~2000 | ~8 MB     | 2000 pages
100,001-300,000 | ~6000 | ~24 MB    | 6000 pages

Note: Actual size depends on value count per key
      and split frequency.
```

This visual guide complements the detailed research report and implementation guide. Use these diagrams to understand the high-level structure before diving into code implementation.

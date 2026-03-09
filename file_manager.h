#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "node.h"
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>

// FileManager handles all disk I/O operations for the B+ tree
class FileManager {
private:
    std::string filename;
    std::fstream file;
    int next_page_id;  // Next available page ID
    std::vector<int> free_pages;  // List of freed page IDs for reuse
    
    // File header structure (stored at beginning of file)
    struct FileHeader {
        int magic_number;      // Magic number to identify file format
        int next_page_id;      // Next available page ID
        int root_page_id;      // Page ID of root node (-1 if empty tree)
        int num_free_pages;    // Number of free pages
        
        FileHeader() : magic_number(0x42505442), next_page_id(1), 
                      root_page_id(-1), num_free_pages(0) {}
    };
    
    FileHeader header;
    
    // LRU Cache structures
    struct CacheEntry {
        int page_id;
        Node* node;
        bool is_dirty;  // Track if node has been modified
        
        CacheEntry(int pid, Node* n, bool dirty = false) 
            : page_id(pid), node(n), is_dirty(dirty) {}
    };
    
    // LRU cache: list for ordering (MRU at front, LRU at back)
    std::list<CacheEntry> cache_list;
    
    // Map from page_id to iterator in cache_list for O(1) access
    std::unordered_map<int, std::list<CacheEntry>::iterator> cache_map;
    
    static constexpr size_t MAX_CACHE_SIZE = 800;  // Cache up to 800 nodes (~3.2 MB)
    
    // Cache statistics
    mutable size_t cache_hits = 0;
    mutable size_t cache_misses = 0;
    mutable size_t cache_evictions = 0;
    
    // Read header from file
    void readHeader();
    
    // Write header to file
    void writeHeader();
    
    // Internal method to read node from disk (bypasses cache)
    Node* readNodeFromDisk(int page_id);
    
    // Internal method to write node to disk (bypasses cache)
    void writeNodeToDisk(Node* node);
    
    // Evict least recently used node from cache
    void evictLRU();
    
    // Mark a cached node as dirty
    void markDirty(int page_id);
    
public:
    FileManager(const std::string& filename);
    ~FileManager() noexcept;
    
    // Initialize or open existing database file
    void open();
    
    // Close the database file
    void close();
    
    // Allocate a new page and return its ID
    int allocatePage();
    
    // Free a page for reuse
    void freePage(int page_id);
    
    // Read a node from disk
    Node* readNode(int page_id);
    
    // Write a node to disk
    void writeNode(Node* node);
    
    // Get the root page ID
    int getRootPageId() const { return header.root_page_id; }
    
    // Set the root page ID
    void setRootPageId(int page_id);
    
    // Flush all changes to disk (writes all dirty cached nodes)
    void flush();
    
    // Clear the cache (writes dirty nodes first)
    void clearCache();
    
    // Get cache statistics
    void getCacheStats(size_t& hits, size_t& misses, size_t& evictions) const {
        hits = cache_hits;
        misses = cache_misses;
        evictions = cache_evictions;
    }
    
    // Print cache statistics
    void printCacheStats() const;
};

#endif // FILE_MANAGER_H

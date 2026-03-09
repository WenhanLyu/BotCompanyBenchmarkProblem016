#include "file_manager.h"
#include <cstring>
#include <stdexcept>
#include <sys/stat.h>

FileManager::FileManager(const std::string& fname) : filename(fname) {
    next_page_id = 1;
}

FileManager::~FileManager() {
    // Destructor must not throw - wrap calls in try-catch
    try {
        close();
    } catch (const std::exception& e) {
        // Swallow exception - cannot throw from destructor
        // Error during close is acceptable in destructor context
    }
    
    try {
        clearCache();
    } catch (const std::exception& e) {
        // Swallow exception - cannot throw from destructor
        // Cache cleanup failure is acceptable in destructor context
    }
}

void FileManager::open() {
    // Check if file exists
    struct stat buffer;
    bool file_exists = (stat(filename.c_str(), &buffer) == 0);
    
    if (file_exists) {
        // Open existing file
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open database file");
        }
        readHeader();
    } else {
        // Create new file
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to create database file");
        }
        
        // Initialize header
        header = FileHeader();
        writeHeader();
    }
}

void FileManager::close() {
    if (file.is_open()) {
        flush();  // Flush all cached changes
        writeHeader();  // Save header before closing
        file.close();
    }
}

void FileManager::readHeader() {
    file.seekg(0, std::ios::beg);
    if (!file.good()) {
        throw std::runtime_error("Failed to seek to beginning of file for header read");
    }
    
    file.read(reinterpret_cast<char*>(&header.magic_number), sizeof(int));
    if (!file.good()) {
        throw std::runtime_error("Failed to read magic number from file header");
    }
    
    // Validate magic number to detect corrupted files
    if (header.magic_number != 0x42505442) {
        throw std::runtime_error("Invalid magic number: file may be corrupted or not a valid B+ tree database");
    }
    
    file.read(reinterpret_cast<char*>(&header.next_page_id), sizeof(int));
    file.read(reinterpret_cast<char*>(&header.root_page_id), sizeof(int));
    file.read(reinterpret_cast<char*>(&header.num_free_pages), sizeof(int));
    
    if (!file.good()) {
        throw std::runtime_error("Failed to read file header");
    }
    
    // Validate header values to prevent corruption issues
    if (header.next_page_id < 1) {
        header.next_page_id = 1;  // Fix corrupted next_page_id
    }
    
    if (header.root_page_id < -1) {
        header.root_page_id = -1;  // Fix corrupted root_page_id
    }
    
    if (header.num_free_pages < 0 || header.num_free_pages > 100000) {
        // Unreasonable number of free pages, reset to safe state
        header.num_free_pages = 0;
        free_pages.clear();
        return;
    }
    
    // Read free pages list
    free_pages.clear();
    for (int i = 0; i < header.num_free_pages; i++) {
        int page_id;
        file.read(reinterpret_cast<char*>(&page_id), sizeof(int));
        if (!file.good()) {
            // File read error, truncate free pages list at this point
            break;
        }
        free_pages.push_back(page_id);
    }
    
    next_page_id = header.next_page_id;
}

void FileManager::writeHeader() {
    file.seekp(0, std::ios::beg);
    if (!file.good()) {
        throw std::runtime_error("Failed to seek to beginning of file for header write");
    }
    
    // Update header
    header.next_page_id = next_page_id;
    header.num_free_pages = free_pages.size();
    
    // Write header fields
    file.write(reinterpret_cast<const char*>(&header.magic_number), sizeof(int));
    if (!file.good()) {
        throw std::runtime_error("Failed to write magic number: disk may be full or file write error");
    }
    
    file.write(reinterpret_cast<const char*>(&header.next_page_id), sizeof(int));
    file.write(reinterpret_cast<const char*>(&header.root_page_id), sizeof(int));
    file.write(reinterpret_cast<const char*>(&header.num_free_pages), sizeof(int));
    
    if (!file.good()) {
        throw std::runtime_error("Failed to write header fields: disk may be full or file write error");
    }
    
    // Write free pages list
    for (int page_id : free_pages) {
        file.write(reinterpret_cast<const char*>(&page_id), sizeof(int));
        if (!file.good()) {
            throw std::runtime_error("Failed to write free pages list: disk may be full or file write error");
        }
    }
    
    file.flush();
    if (!file.good()) {
        throw std::runtime_error("Failed to flush header: disk may be full or file write error");
    }
}

int FileManager::allocatePage() {
    int page_id;
    
    if (!free_pages.empty()) {
        // Reuse a freed page
        page_id = free_pages.back();
        free_pages.pop_back();
    } else {
        // Allocate a new page
        page_id = next_page_id++;
    }
    
    return page_id;
}

void FileManager::freePage(int page_id) {
    free_pages.push_back(page_id);
}

Node* FileManager::readNode(int page_id) {
    if (page_id < 0) {
        return nullptr;
    }
    
    // Check if node is in cache
    auto it = cache_map.find(page_id);
    if (it != cache_map.end()) {
        // Cache hit - move to front (most recently used) and return direct pointer
        cache_hits++;
        auto list_it = it->second;
        cache_list.splice(cache_list.begin(), cache_list, list_it);
        return list_it->node;
    }
    
    // Cache miss - read from disk
    cache_misses++;
    Node* node = readNodeFromDisk(page_id);
    
    // Add to cache (FileManager owns this node)
    if (cache_list.size() >= MAX_CACHE_SIZE) {
        evictLRU();
    }
    
    cache_list.emplace_front(page_id, node, false);
    cache_map[page_id] = cache_list.begin();
    
    return node;
}

Node* FileManager::readNodeFromDisk(int page_id) {
    // Validate page_id
    if (page_id < 0) {
        throw std::runtime_error("Invalid page_id: cannot read negative page ID");
    }
    
    // Calculate file offset (header size + page_id * PAGE_SIZE)
    // Header size: 4 ints + max free pages list
    int header_size = PAGE_SIZE;  // Reserve first page for header
    long offset = header_size + (long)page_id * PAGE_SIZE;
    
    // Read page from file
    char buffer[PAGE_SIZE];
    file.seekg(offset, std::ios::beg);
    if (!file.good()) {
        throw std::runtime_error("Failed to seek to page for reading");
    }
    
    file.read(buffer, PAGE_SIZE);
    if (!file.good()) {
        // Check if we hit EOF or other read error
        if (file.eof()) {
            throw std::runtime_error("Unexpected EOF while reading node: file may be corrupted");
        } else {
            throw std::runtime_error("Failed to read node from disk");
        }
    }
    
    // Determine node type
    char node_type = buffer[0];
    Node* node = nullptr;
    
    if (node_type == 0) {
        // Internal node
        node = new InternalNode();
    } else if (node_type == 1) {
        // Leaf node
        node = new LeafNode();
    } else {
        // Invalid node type - corrupted data
        throw std::runtime_error("Invalid node type in file: data may be corrupted");
    }
    
    node->page_id = page_id;
    node->deserialize(buffer);
    
    return node;
}

void FileManager::writeNode(Node* node) {
    if (node->page_id < 0) {
        // Node doesn't have a page ID yet, allocate one
        node->page_id = allocatePage();
    }
    
    // Check if node is in cache
    auto it = cache_map.find(node->page_id);
    if (it != cache_map.end()) {
        // Node is already in cache - just mark it as dirty
        it->second->is_dirty = true;
        // Move to front (most recently used)
        cache_list.splice(cache_list.begin(), cache_list, it->second);
    } else {
        // Node not in cache - add it to cache and mark as dirty
        if (cache_list.size() >= MAX_CACHE_SIZE) {
            evictLRU();
        }
        cache_list.emplace_front(node->page_id, node, true);
        cache_map[node->page_id] = cache_list.begin();
    }
}

void FileManager::writeNodeToDisk(Node* node) {
    // Validate node pointer
    if (!node) {
        throw std::runtime_error("Cannot write null node to disk");
    }
    
    // Validate page_id
    if (node->page_id < 0) {
        throw std::runtime_error("Cannot write node with invalid page_id to disk");
    }
    
    // Serialize node to buffer
    char buffer[PAGE_SIZE];
    memset(buffer, 0, PAGE_SIZE);  // Clear buffer
    int bytes_written = node->serialize(buffer);
    
    if (bytes_written > PAGE_SIZE) {
        throw std::runtime_error("Node serialization exceeds page size");
    }
    
    // Calculate file offset
    int header_size = PAGE_SIZE;
    long offset = header_size + (long)node->page_id * PAGE_SIZE;
    
    // Write page to file
    file.seekp(offset, std::ios::beg);
    if (!file.good()) {
        throw std::runtime_error("Failed to seek to page for writing");
    }
    
    file.write(buffer, PAGE_SIZE);
    if (!file.good()) {
        throw std::runtime_error("Failed to write node to disk: disk may be full");
    }
}

void FileManager::setRootPageId(int page_id) {
    header.root_page_id = page_id;
    writeHeader();
}

void FileManager::evictLRU() {
    if (cache_list.empty()) {
        return;
    }
    
    // Get least recently used entry (at the back)
    auto& lru_entry = cache_list.back();
    
    // If dirty, write to disk before evicting
    if (lru_entry.is_dirty) {
        writeNodeToDisk(lru_entry.node);
    }
    
    // Remove from map
    cache_map.erase(lru_entry.page_id);
    
    // Delete the cached node (we own it)
    delete lru_entry.node;
    
    // Remove from cache list
    cache_list.pop_back();
    
    // Track eviction
    cache_evictions++;
}

void FileManager::markDirty(int page_id) {
    auto it = cache_map.find(page_id);
    if (it != cache_map.end()) {
        it->second->is_dirty = true;
    }
}

void FileManager::flush() {
    // Write all dirty nodes to disk
    for (auto& entry : cache_list) {
        if (entry.is_dirty) {
            writeNodeToDisk(entry.node);
            entry.is_dirty = false;
        }
    }
    
    writeHeader();
    file.flush();
    if (!file.good()) {
        throw std::runtime_error("Failed to flush changes to disk: disk may be full or file write error");
    }
}

void FileManager::clearCache() {
    // Flush dirty nodes first and delete all cached nodes
    for (auto& entry : cache_list) {
        if (entry.is_dirty) {
            writeNodeToDisk(entry.node);
        }
        delete entry.node;  // Delete the cached copy
    }
    
    // Clear the cache structures
    cache_list.clear();
    cache_map.clear();
}

void FileManager::printCacheStats() const {
    size_t total_accesses = cache_hits + cache_misses;
    double hit_rate = total_accesses > 0 ? (100.0 * cache_hits / total_accesses) : 0.0;
    
    fprintf(stderr, "=== Cache Statistics ===\n");
    fprintf(stderr, "Cache hits: %zu\n", cache_hits);
    fprintf(stderr, "Cache misses: %zu\n", cache_misses);
    fprintf(stderr, "Total accesses: %zu\n", total_accesses);
    fprintf(stderr, "Hit rate: %.2f%%\n", hit_rate);
    fprintf(stderr, "Cache evictions: %zu\n", cache_evictions);
    fprintf(stderr, "Current cache size: %zu / %zu\n", cache_list.size(), MAX_CACHE_SIZE);
    fprintf(stderr, "=======================\n");
}

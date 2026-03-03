#include "file_manager.h"
#include <cstring>
#include <stdexcept>
#include <sys/stat.h>

FileManager::FileManager(const std::string& fname) : filename(fname) {
    next_page_id = 1;
}

FileManager::~FileManager() {
    close();
    clearCache();
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
    file.read(reinterpret_cast<char*>(&header.magic_number), sizeof(int));
    file.read(reinterpret_cast<char*>(&header.next_page_id), sizeof(int));
    file.read(reinterpret_cast<char*>(&header.root_page_id), sizeof(int));
    file.read(reinterpret_cast<char*>(&header.num_free_pages), sizeof(int));
    
    // Read free pages list
    free_pages.clear();
    for (int i = 0; i < header.num_free_pages; i++) {
        int page_id;
        file.read(reinterpret_cast<char*>(&page_id), sizeof(int));
        free_pages.push_back(page_id);
    }
    
    next_page_id = header.next_page_id;
}

void FileManager::writeHeader() {
    file.seekp(0, std::ios::beg);
    
    // Update header
    header.next_page_id = next_page_id;
    header.num_free_pages = free_pages.size();
    
    // Write header fields
    file.write(reinterpret_cast<const char*>(&header.magic_number), sizeof(int));
    file.write(reinterpret_cast<const char*>(&header.next_page_id), sizeof(int));
    file.write(reinterpret_cast<const char*>(&header.root_page_id), sizeof(int));
    file.write(reinterpret_cast<const char*>(&header.num_free_pages), sizeof(int));
    
    // Write free pages list
    for (int page_id : free_pages) {
        file.write(reinterpret_cast<const char*>(&page_id), sizeof(int));
    }
    
    file.flush();
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
        auto list_it = it->second;
        cache_list.splice(cache_list.begin(), cache_list, list_it);
        return list_it->node;
    }
    
    // Cache miss - read from disk
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
    // Calculate file offset (header size + page_id * PAGE_SIZE)
    // Header size: 4 ints + max free pages list
    int header_size = PAGE_SIZE;  // Reserve first page for header
    long offset = header_size + (long)page_id * PAGE_SIZE;
    
    // Read page from file
    char buffer[PAGE_SIZE];
    file.seekg(offset, std::ios::beg);
    file.read(buffer, PAGE_SIZE);
    
    // Determine node type
    char node_type = buffer[0];
    Node* node = nullptr;
    
    if (node_type == 0) {
        // Internal node
        node = new InternalNode();
    } else {
        // Leaf node
        node = new LeafNode();
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
    file.write(buffer, PAGE_SIZE);
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

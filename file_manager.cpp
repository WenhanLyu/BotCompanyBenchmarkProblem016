#include "file_manager.h"
#include <cstring>
#include <stdexcept>
#include <sys/stat.h>

FileManager::FileManager(const std::string& fname) : filename(fname) {
    next_page_id = 1;
}

FileManager::~FileManager() {
    close();
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

void FileManager::flush() {
    writeHeader();
    file.flush();
}

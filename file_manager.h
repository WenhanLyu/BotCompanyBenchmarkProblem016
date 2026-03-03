#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "node.h"
#include <fstream>
#include <string>
#include <vector>

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
    
    // Read header from file
    void readHeader();
    
    // Write header to file
    void writeHeader();
    
public:
    FileManager(const std::string& filename);
    ~FileManager();
    
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
    
    // Flush all changes to disk
    void flush();
};

#endif // FILE_MANAGER_H

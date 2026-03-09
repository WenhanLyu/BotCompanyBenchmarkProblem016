#include "file_manager.h"
#include <iostream>
#include <type_traits>

int main() {
    std::cout << "FileManager destructor is noexcept: " 
              << std::is_nothrow_destructible<FileManager>::value << std::endl;
    
    // Check if it's explicitly marked noexcept in signature
    // This would be 1 if ~FileManager() noexcept; in header
    std::cout << "Is explicitly noexcept: " 
              << noexcept(std::declval<FileManager>().~FileManager()) << std::endl;
    
    return 0;
}

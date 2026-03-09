#include <iostream>
#include <type_traits>
#include "file_manager.h"

int main() {
    std::cout << "FileManager destructor is noexcept: " 
              << std::is_nothrow_destructible<FileManager>::value << std::endl;
    return 0;
}

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

int main() {
    std::ofstream out("large_test_input.txt");
    
    const int NUM_OPS = 15000;
    out << NUM_OPS << std::endl;
    
    srand(time(NULL));
    
    // Mix of operations
    for (int i = 0; i < 5000; i++) {
        // Insert phase - create some data
        out << "insert key" << (i % 1000) << " " << (1000 + i) << std::endl;
    }
    
    for (int i = 0; i < 3000; i++) {
        // Find phase
        out << "find key" << (rand() % 1000) << std::endl;
    }
    
    for (int i = 0; i < 5000; i++) {
        // More inserts with duplicates
        out << "insert key" << (i % 500) << " " << (2000 + i) << std::endl;
    }
    
    for (int i = 0; i < 1000; i++) {
        // Delete phase
        out << "delete key" << (i % 500) << " " << (1000 + i) << std::endl;
    }
    
    for (int i = 0; i < 1000; i++) {
        // Final find phase
        out << "find key" << (i % 500) << std::endl;
    }
    
    out.close();
    
    std::cout << "Generated " << NUM_OPS << " operations" << std::endl;
    return 0;
}

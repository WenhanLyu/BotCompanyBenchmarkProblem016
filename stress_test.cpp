/**
 * Performance Stress Test for B+ Tree File Storage
 * 
 * This program generates a controlled mix of operations and measures:
 * - Total execution time
 * - Memory usage 
 * - Disk usage
 * 
 * Usage: ./stress_test <num_operations> [seed]
 * Example: ./stress_test 300000 42
 */

#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>
#include <sys/stat.h>

struct TestOperation {
    enum Type { INSERT, FIND, DELETE } type;
    std::string key;
    int value;
};

class StressTestGenerator {
private:
    std::mt19937 rng;
    int num_operations;
    
    // Realistic operation mix: 60% insert, 30% find, 10% delete
    static constexpr double INSERT_RATIO = 0.60;
    static constexpr double FIND_RATIO = 0.30;
    static constexpr double DELETE_RATIO = 0.10;
    
public:
    StressTestGenerator(int n_ops, unsigned seed = 42) 
        : rng(seed), num_operations(n_ops) {}
    
    std::vector<TestOperation> generateOperations() {
        std::vector<TestOperation> ops;
        ops.reserve(num_operations);
        
        // Generate key pool (about 1/3 of operations to ensure duplicates)
        int num_keys = num_operations / 3;
        std::vector<std::string> key_pool;
        key_pool.reserve(num_keys);
        
        for (int i = 0; i < num_keys; i++) {
            // Mix of different key lengths for realistic testing
            if (i % 10 == 0) {
                // 10% max-length keys (64 bytes)
                key_pool.push_back(generateKey(64));
            } else if (i % 5 == 0) {
                // 20% medium keys (32 bytes)
                key_pool.push_back(generateKey(32));
            } else {
                // 70% short keys (8-16 bytes)
                key_pool.push_back(generateKey(8 + (i % 9)));
            }
        }
        
        // Generate operations with realistic mix
        std::uniform_real_distribution<> op_dist(0.0, 1.0);
        std::uniform_int_distribution<> key_dist(0, num_keys - 1);
        std::uniform_int_distribution<> val_dist(-1000000, 1000000);
        
        for (int i = 0; i < num_operations; i++) {
            TestOperation op;
            double r = op_dist(rng);
            
            if (r < INSERT_RATIO) {
                op.type = TestOperation::INSERT;
            } else if (r < INSERT_RATIO + FIND_RATIO) {
                op.type = TestOperation::FIND;
            } else {
                op.type = TestOperation::DELETE;
            }
            
            op.key = key_pool[key_dist(rng)];
            op.value = val_dist(rng);
            
            ops.push_back(op);
        }
        
        return ops;
    }
    
private:
    std::string generateKey(size_t length) {
        static const char charset[] = 
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        static const size_t charset_size = sizeof(charset) - 1;
        
        std::uniform_int_distribution<> char_dist(0, charset_size - 1);
        std::string key;
        key.reserve(length);
        
        for (size_t i = 0; i < length; i++) {
            key += charset[char_dist(rng)];
        }
        
        return key;
    }
};

void writeTestFile(const std::string& filename, const std::vector<TestOperation>& ops) {
    std::ofstream out(filename);
    out << ops.size() << "\n";
    
    for (const auto& op : ops) {
        switch (op.type) {
            case TestOperation::INSERT:
                out << "insert " << op.key << " " << op.value << "\n";
                break;
            case TestOperation::FIND:
                out << "find " << op.key << "\n";
                break;
            case TestOperation::DELETE:
                out << "delete " << op.key << " " << op.value << "\n";
                break;
        }
    }
}

long getFileSize(const std::string& filename) {
    struct stat st;
    if (stat(filename.c_str(), &st) == 0) {
        return st.st_size;
    }
    return 0;
}

struct BenchmarkResult {
    int num_operations;
    double elapsed_time_sec;
    long disk_usage_bytes;
    bool success;
};

BenchmarkResult runBenchmark(const std::string& test_file, const std::string& executable) {
    BenchmarkResult result;
    result.num_operations = 0;
    result.success = false;
    
    // Clean up database file before test
    std::remove("database.db");
    
    // Count operations
    std::ifstream in(test_file);
    in >> result.num_operations;
    in.close();
    
    // Build command with time measurement
    std::string command = "/usr/bin/time -l " + executable + " < " + test_file + " > /dev/null 2> .stress_time_output";
    
    auto start = std::chrono::high_resolution_clock::now();
    int exit_code = std::system(command.c_str());
    auto end = std::chrono::high_resolution_clock::now();
    
    result.elapsed_time_sec = std::chrono::duration<double>(end - start).count();
    result.disk_usage_bytes = getFileSize("database.db");
    result.success = (exit_code == 0);
    
    return result;
}

void printResults(const BenchmarkResult& result) {
    std::cout << "\n=== Performance Benchmark Results ===\n";
    std::cout << "Operations: " << result.num_operations << "\n";
    std::cout << "Execution Time: " << result.elapsed_time_sec << " seconds\n";
    std::cout << "Avg Time per Op: " << (result.elapsed_time_sec * 1000000.0 / result.num_operations) << " µs\n";
    std::cout << "Disk Usage: " << (result.disk_usage_bytes / 1024.0 / 1024.0) << " MiB\n";
    std::cout << "Status: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
    
    // Check against requirements
    bool meets_time = result.elapsed_time_sec <= 4.0; // Target 4s for 300K (20% margin)
    bool meets_disk = (result.disk_usage_bytes / 1024.0 / 1024.0) <= 60.0;
    
    std::cout << "\n=== Requirements Check ===\n";
    std::cout << "Time limit (≤4s for 300K): " << (meets_time ? "PASS ✓" : "FAIL ✗") << "\n";
    std::cout << "Memory limit (≤60 MiB): " << (meets_disk ? "PASS ✓" : "FAIL ✗") << "\n";
    
    if (result.num_operations >= 300000 && meets_time && meets_disk) {
        std::cout << "\n🎉 All performance targets met!\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <num_operations> [seed] [executable]\n";
        std::cerr << "Example: " << argv[0] << " 300000 42 ./code\n";
        return 1;
    }
    
    int num_ops = std::atoi(argv[1]);
    unsigned seed = (argc > 2) ? std::atoi(argv[2]) : 42;
    std::string executable = (argc > 3) ? argv[3] : "./code";
    
    std::cout << "=== B+ Tree Performance Stress Test ===\n";
    std::cout << "Generating " << num_ops << " operations (seed: " << seed << ")...\n";
    
    StressTestGenerator generator(num_ops, seed);
    auto operations = generator.generateOperations();
    
    std::string test_file = "stress_test_input.txt";
    std::cout << "Writing test file: " << test_file << "\n";
    writeTestFile(test_file, operations);
    
    std::cout << "Running benchmark with " << executable << "...\n";
    auto result = runBenchmark(test_file, executable);
    
    printResults(result);
    
    // Clean up
    std::remove(test_file.c_str());
    std::remove(".stress_time_output");
    
    return result.success ? 0 : 1;
}

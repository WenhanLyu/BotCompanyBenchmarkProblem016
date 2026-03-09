#include <iostream>
#include <random>
using namespace std;

int main() {
    mt19937 gen(42);
    uniform_int_distribution<> dist(1, 100000);
    
    cout << "300000\n";
    
    // 200K inserts
    for (int i = 0; i < 200000; i++) {
        cout << "insert key" << dist(gen) << " " << dist(gen) << "\n";
    }
    
    // 50K finds
    for (int i = 0; i < 50000; i++) {
        cout << "find key" << dist(gen) << "\n";
    }
    
    // 50K deletes
    for (int i = 0; i < 50000; i++) {
        cout << "delete key" << dist(gen) << " " << dist(gen) << "\n";
    }
    
    return 0;
}

#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>

// Function that creates memory leak when enabled
void create_memory_leak() {
    const size_t MB = 1024 * 1024; // 1 MB
    int leak_counter = 0;
    
    std::cout << "Starting memory allocation (with leak)..." << std::endl;
    std::cout << "Press Ctrl+C to stop the program." << std::endl;
    
    while (true) {
        // Allocate 1 MB of memory
        char* memory_block = new char[MB];
        
        // Write some data to prevent optimization
        for (size_t i = 0; i < MB; i += 1024) {
            memory_block[i] = static_cast<char>(i % 256);
        }
        
        leak_counter++;
        std::cout << "Allocated " << leak_counter << " MB ";
        
        // TO CREATE MEMORY LEAK: COMMENT OUT THE NEXT LINE
        // TO FIX MEMORY LEAK: UNCOMMENT THE NEXT LINE
       // delete[] memory_block;  // ← COMMENT/UNCOMMENT THIS LINE TO TOGGLE LEAK
        
        if (memory_block == NULL) { // Check if deleted
            std::cout << "(memory freed)" << std::endl;
        } else {
            std::cout << "(LEAKING - memory not freed)" << std::endl;
        }
        
        sleep(1); // Wait 1 second
    }
}

// Alternative version using vector (no leak possible)
void no_memory_leak() {
    const size_t MB = 1024 * 1024; // 1 MB
    int allocation_counter = 0;
    
    std::cout << "Starting memory allocation (no leak - using vector)..." << std::endl;
    std::cout << "Press Ctrl+C to stop the program." << std::endl;
    
    while (true) {
        // Using vector for automatic memory management
        std::vector<char> memory_block(MB);
        
        // Write some data to prevent optimization
        for (size_t i = 0; i < MB; i += 1024) {
            memory_block[i] = static_cast<char>(i % 256);
        }
        
        allocation_counter++;
        std::cout << "Allocated and freed " << allocation_counter << " MB (no leak - vector)" << std::endl;
        
        // Memory automatically released when vector goes out of scope
        sleep(1); // Wait 1 second
    }
}

// Simple version without menu - just run the leak
void simple_memory_leak() {
    const size_t MB = 1024 * 1024; // 1 MB
    int counter = 0;
    
    std::cout << "=== SIMPLE MEMORY LEAK DEMO ===" << std::endl;
    std::cout << "Allocating 1MB per second..." << std::endl;
    
    // TO CREATE LEAK: Comment out the delete[] line below
    // TO FIX LEAK: Uncomment the delete[] line below
    
    while (true) {
        char* block = new char[MB];
        
        // Use the memory
        memset(block, counter % 256, MB);
        
        counter++;
        std::cout << "Allocated " << counter << " MB total" << std::endl;
        
        // DELETE[] LINE - COMMENT/UNCOMMENT THIS:
        delete[] block;  // ← COMMENT THIS OUT TO SEE MEMORY LEAK
        
        sleep(1);
    }
}

int main() {
    std::cout << "=== Memory Leak Demonstration ===" << std::endl;
    std::cout << "Choose mode:" << std::endl;
    std::cout << "1 - Memory leak version (comment delete[] to see leak)" << std::endl;
    std::cout << "2 - No leak version (using vector)" << std::endl;
    std::cout << "3 - Simple version (easy to toggle leak)" << std::endl;
    std::cout << "Enter choice (1, 2, or 3): ";
    
    int choice;
    std::cin >> choice;
    
    if (choice == 1) {
        create_memory_leak();
    } else if (choice == 2) {
        no_memory_leak();
    } else if (choice == 3) {
        simple_memory_leak();
    } else {
        std::cout << "Invalid choice. Exiting." << std::endl;
        return 1;
    }
    
    return 0;
}
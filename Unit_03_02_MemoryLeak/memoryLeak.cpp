#include <iostream>
#include <vector>
#include <thread>
#include <csignal>
#include <chrono>
#include <unistd.h>

// memberMemoryLeak.cpp
// Demonstrates a memory leak caused by heap allocations inside a class member
// Build: g++ -std=c++17 memberMemoryLeak.cpp -o memberMemoryLeak

class Leaky {
public:
    Leaky(size_t size): size_(size) {
        // allocate a buffer on the heap as a member
        data_ = new char[size_];
        // touch memory to ensure pages are committed
        for (size_t i = 0; i < size_; i += 4096) data_[i] = 0xAA;
    }

    // NOTE: destructor intentionally does NOT free `data_` so this object leaks
    // To fix the leak, uncomment the destructor body below (or delete data_ elsewhere).
    ~Leaky() {
        // Free the member allocation so that deleting the object releases its heap memory.
        // To deliberately cause a leak, comment out the following line.
        delete[] data_;
    }

    // helper to inspect
    size_t size() const { return size_; }

private:
    char* data_ = nullptr;
    size_t size_ = 0;
};

static std::atomic<bool> running{true};

void handle_sigint(int) {
    running.store(false);
}

int main() {
    const size_t ONE_MB = 1024 * 1024;
    std::vector<Leaky*> leaks;
    leaks.reserve(1024);

    size_t count = 0;
    std::cout << "memberMemoryLeak: PID=" << getpid() << " starting. Allocating ~1MB per second.\n";
    std::cout << "Ctrl+C to stop.\n";

    // Install SIGINT handler so Ctrl+C stops allocation and allows cleanup
    std::signal(SIGINT, handle_sigint);

    while (running.load()) {
        // create a Leaky object that internally allocates memory
        Leaky* p = new Leaky(ONE_MB);

        // By default we keep the pointer around and never delete it -> leak
        leaks.push_back(p);

        ++count;
        if (count % 10 == 0) std::cout << "Allocated " << count << " MB via Leaky objects" << std::endl;

        // Sleep ~1 second between allocations
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clean up: delete all stored Leaky objects (their destructor frees member memory).
    for (Leaky* p : leaks) delete p;
    leaks.clear();

    std::cout << "Cleanup complete, exiting." << std::endl;

    return 0;
}

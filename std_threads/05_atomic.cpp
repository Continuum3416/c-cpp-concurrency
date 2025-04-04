// std::atomic - Lock-Free Operations
// Concept: Using atomic types for simple operations like counting, avoiding mutex overhead.

#include <iostream>
#include <thread>
#include <vector>
#include <atomic> // Include atomic header

// Use std::atomic<int> instead of int and mutex
std::atomic<int> atomic_counter = 0;
const int ITERATIONS_ATOMIC = 100000;

void atomic_increment() {
    for (int i = 0; i < ITERATIONS_ATOMIC; ++i) {
        // This increment operation is atomic.
        // No explicit locking needed. Often faster than mutex for simple ops.
        atomic_counter++; // Or atomic_counter.fetch_add(1);
    }
}

int main() {
    const int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    atomic_counter = 0; // Reset

    std::cout << "Expected counter value: " << NUM_THREADS * ITERATIONS_ATOMIC << std::endl;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(atomic_increment);
    }

    for (std::thread& t : threads) {
        t.join();
    }

    // Should be correct, potentially faster execution
    std::cout << "Actual counter value (atomic): " << atomic_counter << std::endl;

    return 0;
}
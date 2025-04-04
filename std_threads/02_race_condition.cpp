// Race Condition Demonstration (Without Synchronization)
// Concept: Showing how concurrent modification of shared data without protection leads to incorrect results.

#include <iostream>
#include <thread>
#include <vector>

int shared_counter = 0; // Shared data
const int ITERATIONS = 100000;

void unsafe_increment() {
    for (int i = 0; i < ITERATIONS; ++i) {
        // Race condition here! Read-modify-write is not atomic.
        // Multiple threads might read the same value, increment it,
        // and write back, overwriting each other's increments.
        shared_counter++;
    }
}

int main() {
    const int NUM_THREADS = 4;
    std::vector<std::thread> threads;

    std::cout << "Expected counter value: " << NUM_THREADS * ITERATIONS << std::endl;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(unsafe_increment); // Launch threads
    }

    for (std::thread& t : threads) {
        t.join(); // Wait for all threads
    }

    // The final value will likely be LESS than expected due to the race condition
    std::cout << "Actual counter value (unsafe): " << shared_counter << std::endl;

    return 0;
}
// std::mutex - Basic Manual Locking
// Concept: Using a mutex with manual lock() and unlock() to protect the critical section. 
// (Note: Prefer std::lock_guard or std::unique_lock!)

#include <iostream>
#include <thread>
#include <vector>
#include <mutex> // Include mutex header

std::mutex counter_mutex; // Mutex to protect the counter
int shared_counter_mutex = 0;
const int ITERATIONS_MUTEX = 100000;


void safe_increment_manual_lock() {
    for (int i = 0; i < ITERATIONS_MUTEX; ++i) {
        counter_mutex.lock(); // Acquire the lock
        // --- Critical Section Start ---
        shared_counter_mutex++;
        // --- Critical Section End ---
        counter_mutex.unlock(); // Release the lock (MUST be called!)
                                // Potential issue: if an exception occurs before unlock()
    }
}

int main() {
    const int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    shared_counter_mutex = 0; // Reset counter

    std::cout << "Expected counter value: " << NUM_THREADS * ITERATIONS_MUTEX << std::endl;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(safe_increment_manual_lock);
    }

    for (std::thread& t : threads) {
        t.join();
    }

    // Should now be correct
    std::cout << "Actual counter value (manual mutex): " << shared_counter_mutex << std::endl;

    return 0;
}
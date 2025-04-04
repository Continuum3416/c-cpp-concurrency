// std::lock_guard - RAII Locking
// Concept: Using std::lock_guard for automatic, exception-safe mutex locking and unlocking. 
// This is the preferred way for simple scope-based locking.
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

std::mutex counter_mutex_lg; // Mutex
int shared_counter_lg = 0;
const int ITERATIONS_LG = 100000;

void safe_increment_lock_guard() {
    for (int i = 0; i < ITERATIONS_LG; ++i) {
        // Lock is acquired when 'guard' is created
        std::lock_guard<std::mutex> guard(counter_mutex_lg);

        // --- Critical Section Start ---
        shared_counter_lg++;
        // --- Critical Section End ---

        // Lock is automatically released when 'guard' goes out of scope (end of loop iteration)
        // This happens even if an exception is thrown inside the critical section.
    }
}

int main() {
    const int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    shared_counter_lg = 0; // Reset

    std::cout << "Expected counter value: " << NUM_THREADS * ITERATIONS_LG << std::endl;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(safe_increment_lock_guard);
    }

    for (std::thread& t : threads) {
        t.join();
    }

    // Should be correct and safer
    std::cout << "Actual counter value (lock_guard): " << shared_counter_lg << std::endl;

    return 0;
}
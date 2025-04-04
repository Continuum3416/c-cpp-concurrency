// C++20 Synchronization Primitives
// Concept: std::counting_semaphore, std::latch, std::barrier.
// Requirement: C++20 compiler and standard library.

// Requires C++20
#include <iostream>
#include <thread>
#include <vector>
#include <semaphore> // Include semaphore header (C++20)
#include <latch>     // Include latch header (C++20)
#include <barrier>   // Include barrier header (C++20)
#include <chrono>
#include <functional> // For std::function in barrier

// --- Semaphore Example ---
std::counting_semaphore resource_semaphore(2); // Max 2 threads can access resource

void worker_semaphore(int id) {
    std::cout << "[Semaphore] Worker " << id << " waiting to acquire resource..." << std::endl;
    resource_semaphore.acquire(); // Wait if count is 0, then decrement count
    std::cout << "[Semaphore] Worker " << id << " ACQUIRED resource." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate using resource
    std::cout << "[Semaphore] Worker " << id << " releasing resource." << std::endl;
    resource_semaphore.release(); // Increment count
}

// --- Latch Example ---
const int LATCH_COUNT = 3;
std::latch work_latch(LATCH_COUNT); // Initialize latch with count

void worker_latch(int id) {
    std::cout << "[Latch] Worker " << id << " doing preliminary work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    std::cout << "[Latch] Worker " << id << " arrived at latch." << std::endl;
    work_latch.count_down_and_wait(); // Decrement count and wait until it reaches 0
    // work_latch.arrive_and_wait(); // Equivalent shorthand in C++20
    std::cout << "[Latch] Worker " << id << " proceeding past latch." << std::endl;
}

// --- Barrier Example ---
const int BARRIER_COUNT = 3;
// Completion function called by one thread when barrier count is reached
auto barrier_completion = []() noexcept {
    // Note: Avoid blocking or throwing exceptions here
    std::cout << "\n[Barrier] >>> Phase completed! <<<\n" << std::endl;
};
std::barrier work_barrier(BARRIER_COUNT, barrier_completion); // Initialize barrier

void worker_barrier(int id) {
     for (int phase = 0; phase < 2; ++phase) {
        std::cout << "[Barrier] Worker " << id << " starting phase " << phase << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50 * id + 75 * phase));
        std::cout << "[Barrier] Worker " << id << " arriving at barrier for phase " << phase << std::endl;
        work_barrier.arrive_and_wait(); // Arrive, wait for others, completion fn runs, barrier resets
        std::cout << "[Barrier] Worker " << id << " proceeding past barrier for phase " << phase << std::endl;
     }
}


int main() {
    std::vector<std::thread> threads;

    std::cout << "--- Starting Semaphore Example (C++20) ---" << std::endl;
    for (int i = 0; i < 4; ++i) { // Start 4 workers, but semaphore limits to 2
        threads.emplace_back(worker_semaphore, i + 1);
    }
    for (auto& t : threads) t.join();
    threads.clear();
    std::cout << "--- Semaphore Example Finished ---\n" << std::endl;


    std::cout << "--- Starting Latch Example (C++20) ---" << std::endl;
    for (int i = 0; i < LATCH_COUNT; ++i) {
        threads.emplace_back(worker_latch, i + 1);
    }
    for (auto& t : threads) t.join();
    threads.clear();
     std::cout << "--- Latch Example Finished ---\n" << std::endl;


    std::cout << "--- Starting Barrier Example (C++20) ---" << std::endl;
    for (int i = 0; i < BARRIER_COUNT; ++i) {
        threads.emplace_back(worker_barrier, i + 1);
    }
    for (auto& t : threads) t.join();
    threads.clear();
    std::cout << "--- Barrier Example Finished ---" << std::endl;

    return 0;
}
// Compile with: g++ your_code.cpp -o your_executable -pthread -std=c++20
// False Sharing (Illustrative Structure)
// Concept: Demonstrating data layout prone to false sharing. 
// Actually observing the performance impact requires careful benchmarking on specific hardware.

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono> // For timing (in a real benchmark)
#include <new>    // For std::hardware_destructive_interference_size (C++17)

// Structure potentially prone to false sharing if cache line size is 64 bytes
// and threads access adjacent counters.
struct Counters {
    // If CounterA and CounterB are accessed by different threads frequently,
    // modifications to one might invalidate the cache line for the other.
    alignas(64) std::atomic<long long> CounterA = 0; // alignas might help sometimes
    // NO PADDING HERE - CounterB likely shares cache line with CounterA
    std::atomic<long long> CounterB = 0;
};

// Structure with padding to *potentially* mitigate false sharing
// We use the standard constant if available (C++17), otherwise guess 64.
#ifdef __cpp_lib_hardware_interference_size
    constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#else
    constexpr size_t cache_line_size = 64; // Common guess
#endif

struct PaddedCounters {
    alignas(cache_line_size) std::atomic<long long> CounterA = 0;
    // Add padding to push CounterB onto a different cache line
    char padding[cache_line_size - sizeof(std::atomic<long long>)];
    alignas(cache_line_size) std::atomic<long long> CounterB = 0;
};

const long long ITERATIONS_FS = 100'000'000; // Large number for effect

void worker_A(std::atomic<long long>& counter) {
    for (long long i = 0; i < ITERATIONS_FS; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed); // Use relaxed for perf focus
    }
}

void worker_B(std::atomic<long long>& counter) {
     for (long long i = 0; i < ITERATIONS_FS; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    std::cout << "False Sharing Illustration Structure." << std::endl;
    std::cout << "Cache line size guess/constant: " << cache_line_size << std::endl;
    std::cout << "Iterations per thread: " << ITERATIONS_FS << std::endl;

    // --- Test susceptible structure ---
    Counters counters_unpadded;
    std::cout << "\nTesting unpadded structure (potentially susceptible)..." << std::endl;
    auto start1 = std::chrono::high_resolution_clock::now();
    std::thread tA1(worker_A, std::ref(counters_unpadded.CounterA));
    std::thread tB1(worker_B, std::ref(counters_unpadded.CounterB));
    tA1.join();
    tB1.join();
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration1 = end1 - start1;
    std::cout << "Unpadded duration: " << duration1.count() << " ms" << std::endl;
    std::cout << "Unpadded results: A=" << counters_unpadded.CounterA
              << ", B=" << counters_unpadded.CounterB << std::endl;


    // --- Test padded structure ---
    PaddedCounters counters_padded;
     std::cout << "\nTesting padded structure (potentially mitigated)..." << std::endl;
    auto start2 = std::chrono::high_resolution_clock::now();
    std::thread tA2(worker_A, std::ref(counters_padded.CounterA));
    std::thread tB2(worker_B, std::ref(counters_padded.CounterB));
    tA2.join();
    tB2.join();
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration2 = end2 - start2;
    std::cout << "Padded duration: " << duration2.count() << " ms" << std::endl;
    std::cout << "Padded results: A=" << counters_padded.CounterA
              << ", B=" << counters_padded.CounterB << std::endl;

    std::cout << "\nNote: Performance difference depends heavily on CPU architecture and workload." << std::endl;

    return 0;
}
// Compile with: g++ your_code.cpp -o your_executable -pthread -std=c++17 (for alignas/hardware_interference_size)
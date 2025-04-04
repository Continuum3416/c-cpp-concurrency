// thread_local Storage
// Concept: Each thread gets its own independent copy of the variable.

#include <iostream>
#include <thread>
#include <vector>
#include <string>

// Each thread gets its own instance of thread_local_counter and thread_id_str
thread_local int thread_local_counter = 0;
thread_local std::string thread_id_str = "uninitialized";

void worker_thread_local(int id) {
    thread_id_str = "Worker-" + std::to_string(id); // Initialize this thread's copy
    for (int i = 0; i < 5; ++i) {
        thread_local_counter++; // Increments this thread's copy only
        std::cout << "Thread [" << thread_id_str << "] counter = "
                  << thread_local_counter << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main() {
    std::cout << "Starting thread_local example..." << std::endl;
    std::vector<std::thread> threads;

    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(worker_thread_local, i + 1);
    }

    for (std::thread& t : threads) {
        t.join();
    }

    // Accessing thread_local variable from main thread (has its own instance)
    std::cout << "Main thread's counter (usually 0 unless modified by main): "
              << thread_local_counter << std::endl;
     std::cout << "Main thread's ID string: " << thread_id_str << std::endl;


    std::cout << "All threads finished." << std::endl;
    return 0;
}
// Compile with: g++ your_code.cpp -o your_executable -pthread -std=c++11 (or later)
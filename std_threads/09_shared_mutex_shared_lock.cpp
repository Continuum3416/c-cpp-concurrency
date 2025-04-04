// std::shared_mutex and std::shared_lock (Read-Write Lock - C++17)
// Concept: Allows multiple readers OR one writer. Improves performance for read-mostly data.

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <shared_mutex> // Include shared_mutex header (C++17)
#include <mutex> // For std::unique_lock/std::lock_guard if needed
#include <chrono>
#include <map>

std::map<std::string, int> shared_data;
std::shared_mutex data_mutex; // Use shared_mutex

void reader(int id) {
    for (int i = 0; i < 3; ++i) {
        // Acquire shared lock (multiple readers allowed concurrently)
        std::shared_lock<std::shared_mutex> lock(data_mutex);
        std::cout << "Reader " << id << " acquired shared lock. Data size: " << shared_data.size() << std::endl;
        // Simulate reading data
        auto it = shared_data.find("key" + std::to_string(id % 2)); // Example read
        if (it != shared_data.end()) {
            std::cout << "Reader " << id << " read value: " << it->second << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Reader " << id << " releasing shared lock." << std::endl;
        // Shared lock released automatically by RAII (lock going out of scope)
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Small delay between reads
    }
}

void writer(int id) {
    for (int i = 0; i < 2; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50 * id)); // Stagger writers slightly
        // Acquire exclusive lock (only one writer allowed, blocks all readers/writers)
        std::unique_lock<std::shared_mutex> lock(data_mutex);
        std::string key = "key" + std::to_string(id);
        int value = id * 100 + i;
        std::cout << "Writer " << id << " acquired exclusive lock. Writing [" << key << "] = " << value << std::endl;
        shared_data[key] = value;
        // Simulate writing work
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        std::cout << "Writer " << id << " releasing exclusive lock." << std::endl;
        // Exclusive lock released automatically by RAII
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Small delay between writes
    }
}

int main() {
    std::cout << "Starting shared_mutex example (C++17)..." << std::endl;
    std::vector<std::thread> threads;

    // Start multiple readers
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(reader, i + 1);
    }

    // Start a couple of writers
    threads.emplace_back(writer, 1);
    threads.emplace_back(writer, 2);

    for (std::thread& t : threads) {
        t.join();
    }

    std::cout << "All threads finished." << std::endl;
    return 0;
}
// Compile with: g++ your_code.cpp -o your_executable -pthread -std=c++17
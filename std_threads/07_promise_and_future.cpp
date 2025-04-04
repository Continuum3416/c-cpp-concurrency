// std::promise, std::future - Returning Values from Threads
// Concept: Using promise/future pair to send a result from a worker thread back to the parent thread.

#include <iostream>
#include <thread>
#include <future> // Include future header
#include <numeric> // For std::accumulate
#include <vector>
#include <chrono>

// Function to compute sum, result sent via promise
void calculate_sum(std::vector<int>& data, std::promise<long long> result_promise) {
    try {
        long long sum = 0;
        for (int val : data) {
            sum += val;
             std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Simulate work
        }
        // Set the value in the promise, making it available to the future
        result_promise.set_value(sum);
         std::cout << "Worker thread finished calculation." << std::endl;
    } catch (...) {
        // Set an exception in the promise if something goes wrong
        result_promise.set_exception(std::current_exception());
    }
}

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::promise<long long> sum_promise;            // Create promise
    std::future<long long> sum_future = sum_promise.get_future(); // Get future from promise

    std::cout << "Main thread: starting worker thread..." << std::endl;
    // Launch thread, moving the promise into it (promises can't be copied)
    std::thread worker(calculate_sum, std::ref(numbers), std::move(sum_promise));

    std::cout << "Main thread: doing other work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    std::cout << "Main thread: waiting for result from worker..." << std::endl;
    try {
        // Get the result from the future. This will block until the promise is set.
        long long result = sum_future.get(); // Can only call get() once!
        std::cout << "Main thread: received result = " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Main thread: caught exception from worker: " << e.what() << std::endl;
    }

    worker.join(); // Clean up the thread
    std::cout << "Main thread: finished." << std::endl;
    return 0;
}
// Exception Handling via std::promise/std::future
// Concept: Catching an exception in a worker thread and transporting it to the waiting thread.

#include <iostream>
#include <thread>
#include <future> // Include future header
#include <stdexcept> // For standard exceptions
#include <string>

// Function that might throw an exception
void risky_worker(int input, std::promise<std::string> result_promise) {
    try {
        std::cout << "Worker: Started with input " << input << std::endl;
        if (input < 0) {
            // Simulate an error condition
            throw std::invalid_argument("Input cannot be negative");
        }
        if (input == 0) {
             throw std::runtime_error("Input cannot be zero (another error)");
        }

        // Simulate successful work
        std::string result = "Processed input " + std::to_string(input);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Set the successful result
        result_promise.set_value(result);
        std::cout << "Worker: Set value successfully." << std::endl;

    } catch (...) {
        // Catch any exception
        std::cout << "Worker: Caught exception!" << std::endl;
        // Set the exception in the promise
        result_promise.set_exception(std::current_exception());
    }
}

int main() {
    std::cout << "Starting exception handling example..." << std::endl;

    for (int test_input : {10, -5, 0}) {
        std::promise<std::string> worker_promise;
        std::future<std::string> worker_future = worker_promise.get_future();

        std::cout << "\nMain: Launching worker with input: " << test_input << std::endl;
        std::thread worker(risky_worker, test_input, std::move(worker_promise));

        std::cout << "Main: Waiting for result/exception..." << std::endl;
        try {
            // Get the result. If an exception was set, future::get() will re-throw it.
            std::string result = worker_future.get();
            std::cout << "Main: Received result: '" << result << "'" << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Main: Caught std::invalid_argument: " << e.what() << std::endl;
        } catch (const std::runtime_error& e) {
             std::cerr << "Main: Caught std::runtime_error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Main: Caught other std::exception: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Main: Caught unknown exception type!" << std::endl;
        }

        worker.join(); // Always join the thread
        std::cout << "Main: Worker joined." << std::endl;
    }

    std::cout << "\nException handling example finished." << std::endl;
    return 0;
}
// Compile with: g++ your_code.cpp -o your_executable -pthread -std=c++11 (or later)
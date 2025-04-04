// std::async - High-Level Asynchronous Execution
// Concept: A simpler way to launch a potentially asynchronous task and get its result via a std::future. 
// The system decides whether to run it in a new thread or deferred.

#include <iostream>
#include <future> // Include future header
#include <chrono>
#include <string>

// A function that takes time and returns a value
std::string complex_calculation(int input) {
    std::cout << "Async task: Starting calculation for input " << input << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate work
    std::cout << "Async task: Finished calculation." << std::endl;
    return "Result for " + std::to_string(input);
}

int main() {
    std::cout << "Main: Launching async task..." << std::endl;

    // Launch the function asynchronously.
    // std::launch::async policy hints that it should run in a new thread if possible.
    // std::launch::deferred would run it only when get() or wait() is called.
    // Default policy is implementation-defined (often async | deferred).
    std::future<std::string> result_future = std::async(std::launch::async, complex_calculation, 42);

    std::cout << "Main: Async task launched. Doing other work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Main does something else

    std::cout << "Main: Waiting for the result..." << std::endl;

    try {
        // Get the result. This will block if the task hasn't finished yet.
        std::string result = result_future.get();
        std::cout << "Main: Received result: '" << result << "'" << std::endl;
    } catch (const std::exception& e) {
         std::cerr << "Main: Caught exception from async task: " << e.what() << std::endl;
    }


    std::cout << "Main: Finished." << std::endl;
    // No explicit join needed for threads potentially launched by std::async.
    // The future's destructor will wait if necessary (depending on launch policy).
    return 0;
}
// Basic thread creation and joining
// Concept: Launching a function in a separate thread and waiting for it to complete.

#include <iostream>
#include <thread>
#include <string>
#include <chrono> // For sleep

// Function to be executed by the thread
void worker_function(int id, const std::string &message)
{
    std::cout << "Worker thread " << id << " started. Message: " << message << std::endl;

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Work done

    std::cout << "Worker thread " << id << " finished." << std::endl;
}

int main()
{
    std::cout << "Main thread started." << std::endl;

    // Create and launch a new thread
    // Arguments are copied/moved to the new thread's storage
    std::thread t1(worker_function, 1, "Hello from main!");

    // Create and launch another thread using a lambda
    std::thread t2([](int id)
                   {
        std::cout << "Worker thread " << id << " (lambda) started." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Worker thread " << id << " (lambda) finished." << std::endl; }, 2);

    std::cout << "Main thread continues execution..." << std::endl;

    // Wait for the threads to finish before main exits
    // If you don't join (or detach), std::terminate will be called upon thread object destruction
    t1.join();
    t2.join();

    /*
    std::terminate is bad because:
    
    1. std::terminate() is Called: If a std::thread object is destroyed while it is still "joinable" 
    (meaning it represents an active thread of execution that hasn't been joined or detached), 
    its destructor will call std::terminate().

    2. Program Crash: std::terminate() is a function designed to immediately terminate your program abnormally. 
    By default, it usually calls std::abort(), which causes your program to crash, often without proper cleanup 
    (like flushing output streams or releasing resources).

    */

    std::cout << "Main thread finished after joining workers." << std::endl;
    return 0;
}

// g++ -o bin/thread 1_thread.cpp -std=c++17; ./bin/thread
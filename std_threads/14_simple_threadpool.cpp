// Simple Thread Pool (Conceptual Implementation)
// Concept: A basic structure for managing worker threads and a task queue.

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional> // For std::function
#include <future>     // For packaging tasks with return values

class SimpleThreadPool {
public:
    SimpleThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] { // Worker lambda
                while (true) {
                    std::function<void()> task;
                    { // Acquire lock to access queue
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        // Wait until queue is not empty OR stop signal is received
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });

                        // If stop signal received and queue is empty, exit thread
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }

                        // Otherwise, get a task from the queue
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    } // Release lock

                    // Execute the task outside the lock
                    task();
                }
            });
        }
         std::cout << "ThreadPool: Created " << numThreads << " worker threads." << std::endl;
    }

    // Enqueue task using std::function<void()>
    void enqueue(std::function<void()> f) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (stop) { // Don't enqueue if stopping
                 std::cerr << "ThreadPool: Warning! Enqueue on stopped pool." << std::endl;
                 return;
            }
            tasks.emplace(std::move(f));
        }
        condition.notify_one(); // Notify one worker thread
    }


   // Enqueue task that returns a value using std::packaged_task
   // Returns a future to get the result later
   template<class F, class... Args>
   auto enqueue_task(F&& f, Args&&... args)
       -> std::future<typename std::invoke_result<F, Args...>::type> // C++17 invoke_result
   {
       using return_type = typename std::invoke_result<F, Args...>::type;

       // Create a packaged_task which wraps the function and arguments
       auto task_ptr = std::make_shared<std::packaged_task<return_type()>>(
           std::bind(std::forward<F>(f), std::forward<Args>(args)...)
       );

       std::future<return_type> res = task_ptr->get_future(); // Get the future
       {
           std::lock_guard<std::mutex> lock(queue_mutex);
           if(stop) throw std::runtime_error("Enqueue on stopped ThreadPool");

           // Enqueue the packaged_task (type-erased to void())
           tasks.emplace([task_ptr](){ (*task_ptr)(); });
       }
       condition.notify_one();
       return res;
   }


    // Destructor: signal stop, wake all threads, join them
    ~SimpleThreadPool() {
         std::cout << "ThreadPool: Destructor called. Stopping workers..." << std::endl;
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all(); // Wake up all waiting threads
        for (std::thread &worker : workers) {
            if(worker.joinable()) {
                 worker.join();
            }
        }
        std::cout << "ThreadPool: All workers joined. Pool destroyed." << std::endl;
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks; // Queue of tasks

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop; // Flag to signal threads to stop
};

// --- Example Usage ---
int main() {
    SimpleThreadPool pool(4); // Create pool with 4 threads

    std::vector<std::future<int>> results; // To store futures

    // Enqueue void tasks
    for(int i = 0; i < 5; ++i) {
        pool.enqueue([i] { // Enqueue lambda tasks
            std::cout << "Task (void) " << i << " executing in thread "
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            std::cout << "Task (void) " << i << " finished." << std::endl;
        });
    }

    // Enqueue tasks that return values
    for(int i = 0; i < 5; ++i) {
        results.emplace_back(
            pool.enqueue_task([i] { // Enqueue tasks returning int
                std::cout << "Task (ret " << i << ") executing in thread "
                          << std::this_thread::get_id() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                 std::cout << "Task (ret " << i << ") finished calculation." << std::endl;
                return i * i;
            })
        );
    }

     std::cout << "Main: All tasks enqueued. Waiting for results..." << std::endl;

    // Retrieve results from futures
    for(size_t i=0; i < results.size(); ++i) {
        try {
             int result = results[i].get(); // Wait for task to complete and get result
             std::cout << "Main: Result for task " << i << " = " << result << std::endl;
        } catch(const std::exception& e) {
             std::cerr << "Main: Exception getting result for task " << i << ": " << e.what() << '\n';
        }
    }

    std::cout << "Main: All results retrieved. Pool will now destruct." << std::endl;
    // Pool destructor will handle joining threads when 'pool' goes out of scope
    return 0;
}
// Compile with: g++ your_code.cpp -o your_executable -pthread -std=c++17 (for invoke_result)
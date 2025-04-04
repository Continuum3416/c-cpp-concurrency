// Task Queue (Producer/Consumer) with std::condition_variable and std::unique_lock

// Concept: Implementing a thread-safe queue where producers add tasks and consumers process them, using condition variables to signal availability and unique_lock for flexible locking needed by the CV.

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional> // For returning potentially empty values

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> q;
    mutable std::mutex mtx; // Mutex to protect the queue
    std::condition_variable cv_consumer; // Signal for consumers (queue not empty)
    std::condition_variable cv_producer; // Optional: Signal for producers (queue not full)
    size_t max_size; // Optional: bound the queue size
    std::atomic<bool> finished = false; // Flag to signal completion

public:
    ThreadSafeQueue(size_t maxSize = 1000) : max_size(maxSize) {}

    void push(T item) {
        std::unique_lock<std::mutex> lock(mtx);
        // Optional: wait if queue is full
        cv_producer.wait(lock, [this]{ return q.size() < max_size || finished; });
        if (finished) return; // Don't push if finished signal received

        q.push(std::move(item));
        lock.unlock(); // Unlock before notifying to reduce contention
        cv_consumer.notify_one(); // Notify one waiting consumer
    }

    // Try to pop an item, return std::nullopt if queue empty and finished
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mtx);
        // Wait until queue is not empty OR finished flag is set
        cv_consumer.wait(lock, [this]{ return !q.empty() || finished; });

        if (q.empty() && finished) {
            return std::nullopt; // Indicate no more items will arrive
        }
        // Check again after waking up (spurious wakeups or finished flag)
         if (q.empty()) {
             return std::nullopt;
         }


        T item = std::move(q.front());
        q.pop();
        lock.unlock(); // Unlock before notifying
        cv_producer.notify_one(); // Optional: Notify one waiting producer (if bounded)
        return item;
    }

     // Signal that no more items will be pushed
    void set_finished() {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
        // Notify all potentially waiting consumers and producers
        cv_consumer.notify_all();
        cv_producer.notify_all();
    }
};

// --- Example Usage ---

ThreadSafeQueue<int> task_queue(10); // Bounded queue size

void producer(int id) {
    for (int i = 0; i < 5; ++i) {
        int task = id * 100 + i;
        std::cout << "Producer " << id << " pushing task " << task << std::endl;
        task_queue.push(task);
        std::this_thread::sleep_for(std::chrono::milliseconds(10 * id + 5)); // Simulate work
    }
    std::cout << "Producer " << id << " finished." << std::endl;
}

void consumer(int id) {
    while (true) {
        std::cout << "Consumer " << id << " waiting for task..." << std::endl;
        std::optional<int> task = task_queue.pop();

        if (task) {
             std::cout << "Consumer " << id << " processing task " << *task << std::endl;
             std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
        } else {
            // No task returned, and queue is marked finished
             std::cout << "Consumer " << id << " received finish signal and queue empty. Exiting." << std::endl;
            break; // Exit loop
        }
    }
}

int main() {
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Start Consumers first
    for (int i = 0; i < 2; ++i) {
        consumers.emplace_back(consumer, i + 1);
    }

    // Start Producers
    for (int i = 0; i < 3; ++i) {
        producers.emplace_back(producer, i + 1);
    }

    // Wait for producers to finish pushing tasks
    for (std::thread& p : producers) {
        p.join();
    }

    std::cout << "All producers finished. Signaling consumers to finish." << std::endl;
    // Signal that no more tasks will be added
    task_queue.set_finished();

    // Wait for consumers to finish processing remaining tasks and exit
    for (std::thread& c : consumers) {
        c.join();
    }

    std::cout << "All consumers finished." << std::endl;
    return 0;
}
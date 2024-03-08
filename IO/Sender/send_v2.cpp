#include <chrono>
#include <condition_variable>
#include <deque>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// Define a Thread Safe Queue
template <typename T>
class ThreadSafeQueue {
   public:
    ThreadSafeQueue() = default;

    T wait_and_pop() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        T data = queue_.front();
        queue_.pop_front();
        return data;
    }

    void push(T data) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push_back(data);
        cv_.notify_one();
    }

   private:
    std::mutex mtx_;
    std::condition_variable cv_;
    std::deque<T> queue_;
};

// Semaphore
class Semaphore {
   public:
    Semaphore(int count = 0) : count_(count) {}

    inline void notify() {
        std::unique_lock<std::mutex> lock(mtx_);
        count_++;
        cv_.notify_one();
    }

    inline void print() {
        std::unique_lock<std::mutex> lock(mtx_);
        std::cout << "count: " << count_ << std::endl;
    }

    inline void wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        while (count_ == 0) {
            cv_.wait(lock);
        }
        count_--;
    }

   private:
    std::mutex mtx_;
    std::condition_variable cv_;
    int count_;
};

// Semaphore variable.
Semaphore sem(3);  // At most 3 threads can request netowrk IO at the same time.

// A function to simulate sending data to server.
void SendData(int data) {
    sem.wait();  // Request a network IO slot.
    // Pretend to send data to server.
    std::this_thread::sleep_for(std::chrono::seconds(5));  // Simulate network IO.
    sem.print();
    std::cout << "Data sent: " << data << std::endl;
    sem.notify();  // Release the network IO slot.
}

ThreadSafeQueue<int> queue;

void consumerThread() {
    // Need to store the future in a vector to avoid the blocking of the consumer thread.
    // Refer:
    // https://cntransgroup.github.io/EffectiveModernCppChinese/7.TheConcurrencyAPI/item38.html
    std::vector<std::future<void>> futures;
    while (true) {
        auto data = queue.wait_and_pop();
        futures.emplace_back(std::async(std::launch::async, SendData, data));
        std::cout << "Already sent data" << std::endl;
    }
}

void producerThead() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(rand() % 3 + 5));
        queue.push(rand() % 100);
    }
}

int main() {
    std::thread consumer(consumerThread);
    consumer.detach();

    std::thread producers[5];
    for (auto &p : producers) {
        p = std::thread(producerThead);
    }

    for (auto &p : producers) {
        p.join();
    }

    return 0;
}

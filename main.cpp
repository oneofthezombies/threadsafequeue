#include <functional>
#include <iostream>
#include <thread>
#include "ThreadSafeQueue.hpp"

int main() {
    std::atomic_bool isDone{false};
    ThreadSafeQueue<std::function<void()>> queue(isDone);

    std::thread consumer([&queue, &isDone]() {
        while (!isDone)
        {
            std::function<void()> data{};
            if (!queue.tryPop(std::chrono::milliseconds(1000), data)) {
                return;
            }
            data();
        }    
    });

    std::thread producer([&queue]() {
        for (int i = 0; i < 1000; ++i) {
            queue.push([i](){ std::cout << i << std::endl; });
        }    
    });

    // isDone = true;
    consumer.join();
    producer.join();
    return 0;
}

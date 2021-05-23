#include <functional>
#include <iostream>
#include <thread>
#include <vector>
#include "ThreadSafeQueue.hpp"
#include "ThreadSafeMessageQueue.hpp"

int main() {
    std::atomic_bool isDone{false};
    {
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

        consumer.join();
        producer.join();
    }
    {
        enum class MessageType : int32_t {
            A = 0,
            B,
        };

        using queue_type = ThreadSafeMessageQueue<MessageType, std::string>;
        queue_type queue(isDone);
        std::mutex printMutex;
        std::vector<std::thread> consumers;
        auto consumerProcess = [&queue, &isDone, &printMutex](MessageType messageType) {
            auto print = [messageType, &printMutex](queue_type::value_type&& value) {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << "key: " << static_cast<int32_t>(messageType) << ", value: " << value << std::endl;
            };

            while (!isDone)
            {
                queue_type::value_type value;
                if (!queue.tryPop(std::chrono::milliseconds(1000), std::move(messageType), value)) {
                    return;
                }
                
                print(std::move(value));
            }    
        };

        consumers.push_back(std::thread(consumerProcess, MessageType::A));
        consumers.push_back(std::thread(consumerProcess, MessageType::B));

        std::thread producer([&queue]() {
            for (int i = 0; i < 1000; ++i) {
                if (i % 2 == 0) {
                    queue.push(MessageType::A, std::to_string(i));
                } else {
                    queue.push(MessageType::B, std::to_string(i));
                }
            }
        });

        for(auto& consumer : consumers) {
            consumer.join();
        }
        producer.join();
    }

    return 0;
}

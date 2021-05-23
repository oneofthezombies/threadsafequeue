#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>

template<typename KeyType, typename ValueType>
struct ThreadSafeMessageQueue {
    using key_type = KeyType;
    using value_type = ValueType;
    using message_type = std::pair<key_type, value_type>;

    ThreadSafeMessageQueue(std::atomic_bool& isDone) noexcept
        : isDone_(isDone) {
    }

    ~ThreadSafeMessageQueue() noexcept {
        signal_.notify_all();
    }

    void push(key_type&& key, value_type&& value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_back(message_type(std::move(key), std::move(value)));
        }
        signal_.notify_all();
    }

    bool tryPop(std::chrono::milliseconds&& timeout, key_type&& key, value_type& outValue) {
        std::unique_lock<std::mutex> lock(mutex_);
        return signal_.wait_for(lock, std::move(timeout), 
            [this, &key, &outValue]() -> bool {
                if (isDone_) {
                    return false;
                }

                if (queue_.empty()) {
                    return false;
                }

                auto it = std::find_if(queue_.begin(), queue_.end(), 
                    [&key](const message_type& message) -> bool { 
                        return message.first == key;
                    });

                if (it == queue_.end()) {
                    return false;
                }

                outValue = std::move(it->second);
                queue_.erase(it);
                return true;
            });
    }

    size_t size() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool empty() const noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    ThreadSafeMessageQueue(ThreadSafeMessageQueue&&) = delete;
    ThreadSafeMessageQueue& operator=(ThreadSafeMessageQueue&&) = delete;

private:
    mutable std::mutex mutex_;
    std::condition_variable signal_;
    std::list<message_type> queue_;
    std::atomic_bool& isDone_;
};
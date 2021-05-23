#pragma once
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>

template<typename T>
struct ThreadSafeQueue {
    using value_type = T;

    ThreadSafeQueue(std::atomic_bool& isDone)
        : isDone_(isDone) {}

    ~ThreadSafeQueue() {
        signal_.notify_all();
    }

    void push(value_type&& data) noexcept {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push_back(std::move(data));
        }
        signal_.notify_all();
    }

    bool tryPop(std::chrono::milliseconds&& timeout, value_type& outData) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return signal_.wait_for(lock, std::move(timeout), 
            [this, &outData]() -> bool { 
                if (isDone_) {
                    return false;
                }

                if (queue_.empty()) {
                    return false;
                }

                outData = std::move(queue_.front());
                queue_.pop_front();
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

    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

private:
    mutable std::mutex mutex_;
    std::condition_variable signal_;
    std::list<value_type> queue_;
    std::atomic_bool& isDone_;
};

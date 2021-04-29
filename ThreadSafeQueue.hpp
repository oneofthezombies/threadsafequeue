#pragma once
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <queue>
#include <mutex>

template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue(std::atomic_bool& isDone)
        : isDone_(isDone) {}

    void push(T&& data) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(data));
        }
        signal_.notify_one();
    }

    bool tryPop(std::chrono::milliseconds&& timeout, T& outData) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!signal_.wait_for(lock, std::move(timeout), [this](){ return isDone_ || !queue_.empty(); }) || isDone_) {
            return false;
        }

        outData = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

private:
    mutable std::mutex mutex_;
    std::condition_variable signal_;
    std::queue<T> queue_;
    std::atomic_bool& isDone_;
};

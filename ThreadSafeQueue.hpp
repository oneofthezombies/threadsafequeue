#pragma once
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <queue>
#include <mutex>

template<typename T>
struct ThreadSafeQueue {
    ThreadSafeQueue(std::atomic_bool& isDone)
        : isDone_(isDone) {}

    ~ThreadSafeQueue() {
        signal_.notify_all();
    }

    void push(T&& data) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(std::move(data));
        }
        signal_.notify_all();
    }

    bool tryPop(std::chrono::milliseconds&& timeout, T& outData) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!signal_.wait_for(lock, std::move(timeout), [this](){ return isDone_ || !queue_.empty(); })) {
            return false;
        }

        if (isDone_) {
            return false;
        }

        if (queue_.empty()) {
            return false;
        }

        outData = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

private:
    mutable std::mutex mutex_;
    std::condition_variable signal_;
    std::queue<T> queue_;
    std::atomic_bool& isDone_;
};


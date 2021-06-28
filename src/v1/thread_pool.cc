#include "thread_pool.h"

namespace vraft {

ThreadPool::ThreadPool()
    :running_(false) {
}

ThreadPool::~ThreadPool() {
    if (running_) {
        Stop();
    }
}

void
ThreadPool::Start(int worker_threads_num) {
    LOG(INFO) << "thread pool start, have " << worker_threads_num << " worker threads ...";

    assert(worker_threads_num > 0);
    assert(worker_threads_.empty());
    running_ = true;

    worker_threads_.reserve(worker_threads_num);
    for (int i = 0; i < worker_threads_num; ++i) {
        LOG(INFO) << "worker thread " << i << " start ...";
        worker_threads_.emplace_back(
            std::make_unique<std::thread>(&ThreadPool::WorkerFunc, this)
        );
    }
}

void
ThreadPool::Stop() {
    LOG(INFO) << "thread pool stop ...";
    {
        std::unique_lock<std::mutex> guard(mutex_);
        running_ = false;
    }
    cond_not_empty_.notify_all();
    int i = 0;
    for (auto &t : worker_threads_) {
        t->join();
        LOG(INFO) << "worker thread " << i << " stop ...";
        ++i;
    }
}

void
ThreadPool::ProduceOne(Task task) {
    {
        std::lock_guard<std::mutex> guard(mutex_);
        queue_.push(std::move(task));
    }
    cond_not_empty_.notify_one();
}

void
ThreadPool::WorkerFunc() {
    while (running_) {
        ConsumeOne();
    }
}

void
ThreadPool::ConsumeOne() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
        cond_not_empty_.wait(lock);
    }

    Task task;
    assert(!queue_.empty());
    task = queue_.front();
    queue_.pop();
    lock.unlock();

    task();
}

}  // namespace vraft

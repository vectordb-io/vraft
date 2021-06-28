#ifndef __VRAFT_THREAD_POOL_H__
#define __VRAFT_THREAD_POOL_H__

#include <cassert>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <functional>
#include <condition_variable>
#include <glog/logging.h>
#include "status.h"

namespace vraft {

class ThreadPool {
  public:
    using Task = std::function<void ()>;

    ThreadPool();
    ~ThreadPool();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void Start(int worker_threads_num);
    void Stop();
    void ProduceOne(Task task);

  private:
    void ConsumeOne();
    void WorkerFunc();

    std::vector<std::unique_ptr<std::thread>> worker_threads_;
    std::queue<Task> queue_;
    std::mutex mutex_;
    std::condition_variable cond_not_empty_;

    bool running_;
};

}  // namespace vraft

#endif

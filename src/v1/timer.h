#ifndef __VRAFT_TIMER_H__
#define __VRAFT_TIMER_H__

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cassert>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <mutex>
#include <thread>
#include <glog/logging.h>
#include "status.h"

namespace vraft {

using TimerCallBack = std::function<void ()>;

const int EPOLL_SIZE = 100;

struct TimerTask {
    int timerfd_;
    TimerCallBack func_;

    TimerTask(int timerfd, TimerCallBack func)
        :timerfd_(timerfd), func_(func) {
        LOG(INFO) << "TimerTask()" << " timerfd_:" << timerfd_;
    }

    TimerTask() {
        LOG(INFO) << "TimerTask()";
    }

    ~TimerTask() {
        LOG(INFO) << "~TimerTask()";
    }
};

class Timer {
  public:
    Timer();
    ~Timer();
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void MainFunc();

    Status Start();
    Status Stop();

    bool running() const {
        return running_;
    }

    // return timerfd
    int RunAfter(TimerCallBack func, int milliseconds);
    int RunEvery(TimerCallBack func, int milliseconds);

    Status ResetRunAfter(int timerfd, int milliseconds);
    Status ResetRunEvery(int timerfd, int milliseconds);

    void Stop(int timerfd);

  private:
    int SetTimer(TimerCallBack func, struct itimerspec &new_value);
    Status AddTask(std::shared_ptr<TimerTask> t);
    Status DelTask(int timerfd);
    Status Clear();

    std::shared_ptr<TimerTask>
    GetTask(int timerfd);

    std::mutex mu_;
    std::unordered_map<int, std::shared_ptr<TimerTask>> tasks_;

    int epfd_;
    bool running_;
    std::unique_ptr<std::thread> thread_;
};

} // namespace vraft

#endif

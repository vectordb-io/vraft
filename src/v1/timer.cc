#include "timer.h"

namespace vraft {

Timer::Timer()
    :epfd_(-1),
     running_(false) {
}

Timer::~Timer() {
}

Status
Timer::Start() {
    LOG(INFO) << "timer thread start ...";
    //running_ = true;
    thread_ = std::make_unique<std::thread>(&Timer::MainFunc, this);
    return Status::OK();
}

Status
Timer::Stop() {
    running_ = false;
    thread_->join();
    LOG(INFO) << "timer thread stop ...";
    thread_.reset(nullptr);
    return Status::OK();
}

int
Timer::SetTimer(TimerCallBack func, struct itimerspec &new_value) {
    int tfd, ret;
    tfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
    assert(tfd >= 0);

    struct epoll_event event;
    event.data.fd = tfd;
    event.events = EPOLLIN;
    ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, tfd, &event);
    assert(ret == 0);

    ret = timerfd_settime(tfd, 0, &new_value, nullptr);
    assert(ret == 0);

    AddTask(std::make_shared<TimerTask>(tfd, func));
    return tfd;
}

int
Timer::RunAfter(TimerCallBack func, int milliseconds) {
    struct timespec start_time, interval_time;
    struct itimerspec new_value;

    start_time.tv_sec = static_cast<time_t>(milliseconds / 1000);
    start_time.tv_nsec = static_cast<long>((milliseconds % 1000) * 1000 * 1000);

    interval_time.tv_sec = 0;
    interval_time.tv_nsec = 0;

    new_value.it_value = start_time;
    new_value.it_interval = interval_time;

    int tfd = SetTimer(func, new_value);
    assert(tfd >= 0);
    return tfd;
}

Status
Timer::ResetRunAfter(int timerfd, int milliseconds) {
    struct timespec start_time, interval_time;
    struct itimerspec new_value;

    start_time.tv_sec = static_cast<time_t>(milliseconds / 1000);
    start_time.tv_nsec = static_cast<long>((milliseconds % 1000) * 1000 * 1000);

    interval_time.tv_sec = 0;
    interval_time.tv_nsec = 0;

    new_value.it_value = start_time;
    new_value.it_interval = interval_time;

    auto ret = timerfd_settime(timerfd, 0, &new_value, nullptr);
    assert(ret == 0);

    return Status::OK();
}

int
Timer::RunEvery(TimerCallBack func, int milliseconds) {
    struct timespec start_time, interval_time;
    struct itimerspec new_value;

    start_time.tv_sec = static_cast<time_t>(milliseconds / 1000);
    start_time.tv_nsec = static_cast<long>((milliseconds % 1000) * 1000 * 1000);

    interval_time.tv_sec = start_time.tv_sec;
    interval_time.tv_nsec = start_time.tv_nsec;

    new_value.it_value = start_time;
    new_value.it_interval = interval_time;

    int tfd = SetTimer(func, new_value);
    assert(tfd >= 0);
    return tfd;
}

Status
Timer::ResetRunEvery(int timerfd, int milliseconds) {
    struct timespec start_time, interval_time;
    struct itimerspec new_value;

    start_time.tv_sec = static_cast<time_t>(milliseconds / 1000);
    start_time.tv_nsec = static_cast<long>((milliseconds % 1000) * 1000 * 1000);

    interval_time.tv_sec = start_time.tv_sec;
    interval_time.tv_nsec = start_time.tv_nsec;

    new_value.it_value = start_time;
    new_value.it_interval = interval_time;

    auto ret = timerfd_settime(timerfd, 0, &new_value, nullptr);
    assert(ret == 0);

    return Status::OK();
}

void
Timer::Stop(int timerfd) {
    struct timespec start_time, interval_time;
    struct itimerspec new_value;

    start_time.tv_sec = 0;
    start_time.tv_nsec = 0;

    interval_time.tv_sec = 0;
    interval_time.tv_nsec = 0;

    new_value.it_value = start_time;
    new_value.it_interval = interval_time;

    auto ret = timerfd_settime(timerfd, 0, &new_value, nullptr);
    assert(ret == 0);
}

Status
Timer::AddTask(std::shared_ptr<TimerTask> t) {
    std::lock_guard<std::mutex> guard(mu_);
    bool ok = tasks_.insert(std::make_pair(t->timerfd_, t)).second;
    assert(ok);

    return Status::OK();
}

Status
Timer::DelTask(int timerfd) {
    std::lock_guard<std::mutex> guard(mu_);
    tasks_.erase(timerfd);

    return Status::OK();
}

Status
Timer::Clear() {
    std::lock_guard<std::mutex> guard(mu_);
    tasks_.clear();

    return Status::OK();
}

std::shared_ptr<TimerTask>
Timer::GetTask(int timerfd) {
    std::lock_guard<std::mutex> guard(mu_);
    auto it = tasks_.find(timerfd);
    if (it == tasks_.end()) {
        return std::shared_ptr<TimerTask>();
    }
    return it->second;
}

void
Timer::MainFunc() {
    struct epoll_event events[EPOLL_SIZE];
    int nfds;
    Status s;

    epfd_ = epoll_create(EPOLL_SIZE);
    assert(!(epfd_ < 0));

    LOG(INFO) << "timer thread start ...";
    running_ = true;
    while (true) {
        if (!running_) {
            break;
        }

        nfds = epoll_wait(epfd_, events, EPOLL_SIZE, 3000);
        if (nfds == 0) {
            continue;
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].events & EPOLLIN) {
                uint64_t data;
                read(events[i].data.fd, &data, sizeof(uint64_t));
                //LOG(INFO) << "timer ring! read from fd:" << events[i].data.fd << ", data:" << data;
                //LOG(INFO) << "timer ring! timer fd:" << events[i].data.fd;
                auto t = GetTask(events[i].data.fd);
                assert(t);
                t->func_();
            }
        }
    }
    s = Clear();
    assert(s.ok());
    LOG(INFO) << "timer thread stop ...";
}

} // namespace vraft

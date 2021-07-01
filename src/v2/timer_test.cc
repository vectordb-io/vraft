#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <memory>
#include <iostream>
#include <functional>
#include "timer.h"


vraft::Timer t;

void Print(std::string s) {
    printf("%s\n", s.c_str());
    fflush(nullptr);
}

void GetTimer(int tfd) {
    struct itimerspec curr_value;
    int r = timerfd_gettime(tfd, &curr_value);
    assert(r == 0);

    printf("it_interval: %ld %ld, it_value:%ld %ld \n",
           curr_value.it_interval.tv_sec,
           curr_value.it_interval.tv_nsec,
           curr_value.it_value.tv_sec,
           curr_value.it_value.tv_nsec);
}

void GetTimer2(int tfd) {
    time_t tv_sec;
    long tv_nsec;
    auto s = t.TimeLeft(tfd, tv_sec, tv_nsec);
    assert(s.ok());

    printf("%ld %ld \n", tv_sec, tv_nsec);
}

int main(int argc, char **argv) {
    auto s = t.Start();
    assert(s.ok());

    sleep(3);
    //int tfd = t.RunAfter(std::bind(Print, "hello"), 30000);
    int tfd = t.RunEvery(std::bind(Print, "hello"), 30000);
    //t.RunEvery(std::bind(GetTimer, tfd), 1000);
    t.RunEvery(std::bind(GetTimer2, tfd), 1000);

    while (true) {
        sleep(3);
    }

    return 0;
}

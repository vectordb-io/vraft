#include <unistd.h>
#include <sys/syscall.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>
#include "log.h"

#define gettid() (syscall(SYS_gettid))

vraft::Log test_log = std::string("/tmp/test_raft_test_log2");

void AppendLog(int count, int term) {
    for (int i = 0; i < count; ++i) {
        char buf[128];
        snprintf(buf, sizeof(buf), "cmd%d_thread%ld", i, gettid());
        std::string cmd(buf);
        vraft::Entry entry(term, cmd);
        std::string log_str = "append log entry: " + entry.ToString() + "\n";
        std::cout << log_str;
        std::cout.flush();
        test_log.AppendEntry(entry);
    }
}

int
main(int argc, char **argv) {
    auto s = test_log.Init();
    assert(s.ok());

    std::vector<std::thread*> threads;
    for (int i = 0; i < 10; ++i) {
        std::thread *t = new std::thread(std::bind(AppendLog, 5, i*100));
        threads.push_back(t);
    }

    for (auto &t : threads) {
        t->join();
    }

    std::cout << test_log.ToStringPretty() << std::endl;

    return 0;
}

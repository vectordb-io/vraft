#include <unistd.h>
#include <sys/syscall.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <memory>
#include <iostream>
#include <functional>
#include "log.h"

#define gettid() (syscall(SYS_gettid))

vraft::Log test_log = std::string("/tmp/test_raft_test_log");

void AppendLog(int count, int term) {
    for (int i = 0; i < count; ++i) {
        char buf[128];
        snprintf(buf, sizeof(buf), "cmd%d_thread%ld", i, gettid());
        std::string cmd(buf);
        vraft::Entry entry(term, cmd);
        std::cout << "append log entry: " << entry.ToString() << std::endl;
        test_log.AppendEntry(entry);
    }
}

int
main(int argc, char **argv) {
    auto s = test_log.Init();
    assert(s.ok());

    std::cout << test_log.ToStringPretty() << std::endl << std::endl;
    AppendLog(1, 88);
    std::cout << test_log.ToStringPretty() << std::endl << std::endl;

    AppendLog(5, 99);
    std::cout << test_log.ToStringPretty() << std::endl << std::endl;

    AppendLog(5, 200);
    std::cout << test_log.ToStringPretty() << std::endl << std::endl;

    std::cout << "TruncateEntries(7)" << std::endl;
    test_log.TruncateEntries(7);
    std::cout << test_log.ToStringPretty() << std::endl << std::endl;

    std::cout << "TruncateEntries(3)" << std::endl;
    test_log.TruncateEntries(3);
    std::cout << test_log.ToStringPretty() << std::endl << std::endl;

    std::cout << "TruncateEntries(1)" << std::endl;
    test_log.TruncateEntries(1);
    std::cout << test_log.ToStringPretty() << std::endl << std::endl;

    return 0;
}

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <memory>
#include <iostream>
#include <functional>
#include "thread_pool.h"

vraft::ThreadPool thread_pool;

void Print(std::string s) {
    //std::cout << s << std::endl;
    printf("%s\n", s.c_str());
    fflush(nullptr);
}

void f(int i) {
    for (int j = 0; j < 1000; ++j) {
        char buf[64];
        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "thread_%d : task_%d", i, j);
        thread_pool.ProduceOne(std::bind(&Print, std::string(buf)));
    }
}

int main(int argc, char **argv) {

    thread_pool.Start(10);

    std::vector<std::unique_ptr<std::thread>> v;

    for (int i = 0; i < 50; ++i) {
        auto p = std::make_unique<std::thread>(f, i);
        v.push_back(std::move(p));
    }

    for (auto &p : v) {
        //std::cout << p->get_id() << " join" << std::endl;
        p->join();
    }

    while(true) {
        //std::cout << "sleep ..." << std::endl;
        printf("sleep ... \n");
        fflush(nullptr);
        sleep(5);
    }

    //thread_pool.Stop();

    return 0;
}

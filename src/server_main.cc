#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <functional>
#include <glog/logging.h>
#include "env.h"
#include "node.h"
#include "status.h"
#include "config.h"

std::string exe_name;

void PrintHelp() {
    std::cout << std::endl;
    std::cout << "Usage: " << std::endl;

    std::cout << std::endl;
    std::cout << "normal: " << std::endl;
    std::cout << exe_name << " --me=127.0.0.1:38000 --peers=127.0.0.1:38001,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38000" << std::endl;
    std::cout << exe_name << " --me=127.0.0.1:38001 --peers=127.0.0.1:38000,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38001" << std::endl;
    std::cout << exe_name << " --me=127.0.0.1:38002 --peers=127.0.0.1:38000,127.0.0.1:38001 --path=/tmp/vraft/127.0.0.1:38002" << std::endl;
    std::cout << exe_name << " -h" << std::endl;
    std::cout << exe_name << " --help" << std::endl;

    std::cout << std::endl;
    std::cout << "ping: " << std::endl;
    std::cout << exe_name << " --me=127.0.0.1:38000 --peers=127.0.0.1:38001,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38000 --ping" << std::endl;
    std::cout << exe_name << " --me=127.0.0.1:38001 --peers=127.0.0.1:38000,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38001 --ping" << std::endl;
    std::cout << exe_name << " --me=127.0.0.1:38002 --peers=127.0.0.1:38000,127.0.0.1:38001 --path=/tmp/vraft/127.0.0.1:38002 --ping" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    vraft::Status s;
    FLAGS_alsologtostderr = true;
    google::InitGoogleLogging(argv[0]);
    exe_name = std::string(argv[0]);

    if (argc < 2) {
        PrintHelp();
        exit(0);
    }

    s = vraft::Config::GetInstance().Load(argc, argv);
    if (s.ok()) {
        LOG(INFO) << "read config:" << vraft::Config::GetInstance().DebugString();
    } else {
        PrintHelp();
        exit(-1);
    }

    s = vraft::Env::GetInstance().Init();
    assert(s.ok());

    s = vraft::Node::GetInstance().Init();
    assert(s.ok());

    s = vraft::Env::GetInstance().Start();
    assert(s.ok());

    s = vraft::Node::GetInstance().Start();
    assert(s.ok());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}

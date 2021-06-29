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

//std::string exe_name;

int main(int argc, char** argv) {
    vraft::Status s;
    FLAGS_alsologtostderr = true;
    google::InitGoogleLogging(argv[0]);

    s = vraft::Config::GetInstance().Init(argc, argv);
    if (s.ok()) {
        LOG(INFO) << vraft::Config::GetInstance().ToString();
    } else {
        LOG(INFO) << s.ToString();
        assert(0);
    }

    if (argc < 2) {
        vraft::Config::GetInstance().PrintHelp();
        exit(0);
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

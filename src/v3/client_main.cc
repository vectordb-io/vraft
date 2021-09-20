#include <getopt.h>
#include <random>
#include <string>
#include <thread>
#include <iostream>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include "vraft_rpc.grpc.pb.h"


std::default_random_engine random_(time(nullptr));
std::string exe_name;

void Usage() {
    std::cout << std::endl;
    std::cout << "Usage: " << std::endl;
    std::cout << exe_name << " --addr=127.0.0.1:38000 --cmd=get_state" << std::endl;
    std::cout << exe_name << " --addr=127.0.0.1:38000 --cmd=put_entry --param=sm_cmd_1" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char **argv) {
    exe_name = std::string(argv[0]);
    if (argc == 1) {
        Usage();
        exit(-1);
    }

    int option_index, option_value;
    option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"cmd", required_argument, nullptr, 'c'},
        {"param", required_argument, nullptr, 'p'},
        {"addr", required_argument, nullptr, 'a'},
        {nullptr, 0, nullptr, 0}
    };

    std::string cmd, param, addr;
    while ((option_value = getopt_long(argc, argv, "hc:p:a:", long_options, &option_index)) != -1) {
        switch (option_value) {
        case 'c':
            cmd = std::string(optarg);
            break;

        case 'p':
            param = std::string(optarg);
            break;

        case 'a':
            addr = std::string(optarg);
            break;

        case 'h':
            Usage();
            exit(0);

        default:
            Usage();
            exit(0);
        }
    }


    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
    std::unique_ptr<vraft_rpc::VRaft::Stub> stub = vraft_rpc::VRaft::NewStub(channel);
    vraft_rpc::ClientRequest request;
    vraft_rpc::ClientRequestReply reply;
    grpc::ClientContext context;

    if (cmd == "get_state") {
        request.set_cmd("get_state");

    } else if (cmd == "put_entry") {
        request.set_cmd("put_entry");
        request.set_param(param);

    } else {
        std::cout << "cmd: " << cmd << " not supported" << std::endl;
        exit(-1);
    }

    grpc::Status s = stub->RpcClientRequest(&context, request, &reply);
    if (s.ok()) {
        std::cout << "code: " << reply.code() << std::endl;
        std::cout << "msg: " << reply.msg() << std::endl;
        std::cout << "response: " << reply.response() << std::endl;
        std::cout << "leader_hint: " << reply.leader_hint() << std::endl;
    } else {
        std::cout << s.error_message();
    }

    return 0;
}

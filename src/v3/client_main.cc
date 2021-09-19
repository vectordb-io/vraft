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
    std::cout << exe_name << " address" << std::endl;
    std::cout << exe_name << " 127.0.0.1:38000" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char **argv) {
    exe_name = std::string(argv[0]);

    if (argc != 2) {
        Usage();
        exit(-1);
    }
    std::string address = argv[1];

    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    std::unique_ptr<vraft_rpc::VRaft::Stub> stub = vraft_rpc::VRaft::NewStub(channel);

    vraft_rpc::ClientRequest request;
    vraft_rpc::ClientRequestReply reply;

    request.set_cmd("__get_state__");
    grpc::ClientContext context;

    grpc::Status s = stub->RpcClientRequest(&context, request, &reply);
    if (s.ok()) {
        std::cout << reply.response() << std::endl;
    } else {
        std::cout << s.error_message();
    }

    return 0;
}

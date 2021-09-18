#include <random>
#include <string>
#include <thread>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include "vraft_rpc.grpc.pb.h"


std::default_random_engine random_(time(nullptr));


int main(int argc, char **argv) {
    FLAGS_alsologtostderr = true;
    google::InitGoogleLogging(argv[0]);



    return 0;
}

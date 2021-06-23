#ifndef __VRAFT_GRPC_SERVER_H__
#define __VRAFT_GRPC_SERVER_H__

#include <string>
#include <thread>
#include <memory>
#include <functional>
#include <grpcpp/grpcpp.h>
#include "vraft_rpc.grpc.pb.h"
#include "status.h"
#include "config.h"
#include "async_task_called.h"
#include "async_task_call.h"

namespace vraft {

class GrpcServer {
  public:
    GrpcServer();
    GrpcServer(const GrpcServer&) = delete;
    GrpcServer& operator=(const GrpcServer&) = delete;
    ~GrpcServer();

    Status Start();
    Status Stop();
    Status StartService();
    void ThreadAsyncCalled();
    void ThreadAsyncCall();

    void IntendOnPing();
    void OnPing(AsyncTaskOnPing *p);
    Status AsyncPing(const vraft_rpc::Ping &request, const std::string &address, PingFinishCallBack cb);
    Status AsyncRequestVote(const vraft_rpc::RequestVote &request, const std::string &address, RequestVoteFinishCallBack cb);

    void set_on_ping_cb(OnPingCallBack cb) {
        on_ping_cb_ = cb;
    }

    void set_on_request_vote_cb(OnRequestVoteCallBack cb) {
        on_request_vote_cb_ = cb;
    }

  private:
    bool running_;
    OnPingCallBack on_ping_cb_;
    OnRequestVoteCallBack on_request_vote_cb_;

    std::unique_ptr<grpc::CompletionQueue> cq_out_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_in_;
    vraft_rpc::VRaft::AsyncService service_;
    std::unique_ptr<grpc::Server> server_;

    // rpc call in, put it into task queue, wait for response
    std::unique_ptr<std::thread> thread_called_;

    // rpc call out, wait for response
    std::unique_ptr<std::thread> thread_call_;

    // boot thread
    std::unique_ptr<std::thread> boot_thread_;
};

} // namespace vraft

#endif

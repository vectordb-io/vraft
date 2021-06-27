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
#include "async_req_manager.h"

namespace vraft {

class GrpcServer {
  public:
    GrpcServer();
    GrpcServer(const GrpcServer&) = delete;
    GrpcServer& operator=(const GrpcServer&) = delete;
    ~GrpcServer();

    Status Init();
    Status Start();
    Status Stop();
    Status StartService();
    void ThreadAsyncCalled();
    void ThreadAsyncCall();

    void IntendOnPing();
    void OnPing(AsyncTaskOnPing *p);
    Status AsyncPing(const vraft_rpc::Ping &request, const std::string &address, PingFinishCallBack cb);

    void IntendOnClientRequest();
    void OnClientRequest(AsyncTaskOnClientRequest *p);
    Status AsyncClientRequestReply(const vraft_rpc::ClientRequestReply &reply, void *call);

    void IntendOnRequestVote();
    void OnRequestVote(AsyncTaskOnRequestVote *p);
    Status AsyncRequestVote(const vraft_rpc::RequestVote &request, const std::string &address, RequestVoteFinishCallBack cb);

    void IntendOnAppendEntries();
    void OnAppendEntries(AsyncTaskOnAppendEntries *p);
    Status AsyncAppendEntries(const vraft_rpc::AppendEntries &request, const std::string &address, AppendEntriesFinishCallBack cb);

    void set_on_ping_cb(OnPingCallBack cb) {
        on_ping_cb_ = cb;
    }

    void set_on_client_request_cb(OnClientRequestCallBack cb) {
        on_client_request_cb_ = cb;
    }

    void set_on_request_vote_cb(OnRequestVoteCallBack cb) {
        on_request_vote_cb_ = cb;
    }

    void set_on_append_entries_cb(OnAppendEntriesCallBack cb) {
        on_append_entries_cb_ = cb;
    }

  private:
    bool running_;
    OnPingCallBack on_ping_cb_;
    OnClientRequestCallBack on_client_request_cb_;
    OnRequestVoteCallBack on_request_vote_cb_;
    OnAppendEntriesCallBack on_append_entries_cb_;

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

    AsyncReqManager async_req_manager_;
};

} // namespace vraft

#endif

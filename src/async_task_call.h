#ifndef __VRAFT_ASYNC_TASK_CALL_H__
#define __VRAFT_ASYNC_TASK_CALL_H__

#include <string>
#include <thread>
#include <memory>
#include <functional>
#include <grpcpp/grpcpp.h>
#include "vraft_rpc.grpc.pb.h"
#include "status.h"
#include "config.h"

namespace vraft {

class AsyncTaskCall {
  public:
    virtual void Process() = 0;
    virtual grpc::Status GetStatus() = 0;
    virtual ~AsyncTaskCall() {}
};

class AsyncTaskPing : public AsyncTaskCall {
  public:
    vraft_rpc::PingReply reply_;
    grpc::ClientContext ctx_;
    grpc::Status status_;
    std::unique_ptr<grpc::ClientAsyncResponseReader<vraft_rpc::PingReply>> response_reader_;
    PingFinishCallBack cb_;

    ~AsyncTaskPing() {}
    virtual void Process() override;
    virtual grpc::Status GetStatus() override;
};

class AsyncTaskRequestVote : public AsyncTaskCall {
  public:
    vraft_rpc::RequestVoteReply reply_;
    grpc::ClientContext ctx_;
    grpc::Status status_;
    std::unique_ptr<grpc::ClientAsyncResponseReader<vraft_rpc::RequestVoteReply>> response_reader_;
    RequestVoteFinishCallBack cb_;

    ~AsyncTaskRequestVote() {}
    virtual void Process() override;
    virtual grpc::Status GetStatus() override;

};

} // namespace vraft

#endif

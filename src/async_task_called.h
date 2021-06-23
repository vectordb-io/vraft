#ifndef __VRAFT_ASYNC_TASK_CALLED_H__
#define __VRAFT_ASYNC_TASK_CALLED_H__

#include <string>
#include <thread>
#include <memory>
#include <functional>
#include <grpcpp/grpcpp.h>
#include "vraft_rpc.grpc.pb.h"
#include "status.h"
#include "config.h"

namespace vraft {

class AsyncTaskCalled {
  public:
    virtual void Process() = 0;
    virtual ~AsyncTaskCalled() {}
};

class AsyncTaskOnPing : public AsyncTaskCalled {
  public:
    AsyncTaskOnPing(vraft_rpc::VRaft::AsyncService* service,
                    grpc::ServerCompletionQueue* cq)
        :service_(service), cq_in_(cq),
         responder_(&ctx_),
         done_(false) {
    }

    ~AsyncTaskOnPing() {}

    virtual void Process() override;

    vraft_rpc::VRaft::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_in_;
    grpc::ServerContext ctx_;

    grpc::ServerAsyncResponseWriter<vraft_rpc::PingReply> responder_;
    vraft_rpc::Ping request_;
    vraft_rpc::PingReply reply_;

    bool done_;
    OnPingCallBack cb_;
};

} // namespace vraft

#endif

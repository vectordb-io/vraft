#include  <functional>
#include <glog/logging.h>
#include "env.h"
#include "grpc_server.h"
#include "thread_pool.h"
#include "async_task_call.h"

namespace vraft {

grpc::Status
AsyncTaskPing::GetStatus() {
    return status_;
}

void
AsyncTaskPing::Process() {
    auto s = cb_(reply_);
    assert(s.ok());
    // optimizing by memory pool
    delete this;
}

grpc::Status
AsyncTaskRequestVote::GetStatus() {
    return status_;
}

void
AsyncTaskRequestVote::Process() {
    auto s = cb_(reply_);
    assert(s.ok());
    // optimizing by memory pool
    delete this;
}

grpc::Status
AsyncTaskAppendEntries::GetStatus() {
    return status_;
}

void
AsyncTaskAppendEntries::Process() {
    auto s = cb_(reply_);
    assert(s.ok());
    // optimizing by memory pool
    delete this;
}

} // namespace vraft

#include  <functional>
#include <glog/logging.h>
#include "env.h"
#include "async_task_called.h"

namespace vraft {

void
AsyncTaskOnPing::Process() {
    if (done_) {
        // optimizing by memory pool
        delete this;
    } else {
        Env::GetInstance().thread_pool()->ProduceOne(std::bind(&GrpcServer::OnPing, Env::GetInstance().grpc_server(), this));
        Env::GetInstance().grpc_server()->IntendOnPing();
    }
}

} // namespace vraft

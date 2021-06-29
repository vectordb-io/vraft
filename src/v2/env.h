#ifndef __VRAFT_ENV_H__
#define __VRAFT_ENV_H__

#include <chrono>
#include "log.h"
#include "timer.h"
#include "common.h"
#include "status.h"
#include "storage.h"
#include "grpc_server.h"
#include "thread_pool.h"
#include "vraft_rpc.grpc.pb.h"

namespace vraft {

class Env {
  public:
    static Env&
    GetInstance() {
        static Env instance;
        return instance;
    }

    Status Init();
    Status Start();
    Status Stop();

    // network
    Status AsyncPing(const vraft_rpc::Ping &request, const std::string &address, PingFinishCallBack cb);
    Status AsyncRequestVote(const vraft_rpc::RequestVote &request, const std::string &address, RequestVoteFinishCallBack cb);
    Status AsyncAppendEntries(const vraft_rpc::AppendEntries &request, const std::string &address, AppendEntriesFinishCallBack cb);

    // storage
    Status CurrentTerm(int64_t &term) const;
    Status PersistCurrentTerm(int64_t term);
    Status VoteFor(uint64_t &node_id) const;
    Status PersistVoteFor(uint64_t node_id);

    ThreadPool* thread_pool() {
        return &thread_pool_;
    }

    GrpcServer* grpc_server() {
        return &grpc_server_;
    }

    Timer* timer() {
        return &timer_;
    }

  private:
    Env();
    ~Env();
    Env(const Env&) = delete;
    Env& operator=(const Env&) = delete;

    Timer timer_;
    Storage storage_;
    ThreadPool thread_pool_;
    GrpcServer grpc_server_;
};

}  // namespace vraft

#endif

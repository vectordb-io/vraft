#ifndef __VRAFT_NODE_H__
#define __VRAFT_NODE_H__

#include <random>
#include <functional>
#include <glog/logging.h>
#include "raft.h"
#include "status.h"
#include "common.h"
#include "vraft_rpc.grpc.pb.h"

namespace vraft {


class Node {
  public:
    static Node&
    GetInstance() {
        static Node instance;
        return instance;
    }

    Node() = default;
    ~Node() {}
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    Status Init();
    Status Start();
    uint64_t Id();

    void OnPing(const vraft_rpc::Ping &request, vraft_rpc::PingReply &reply);
    Status Ping(const vraft_rpc::Ping &request, const std::string &address);
    Status OnPingReply(const vraft_rpc::PingReply &reply);
    Status PingAll();
    Status PingPeers();

    void Sleep(int min, int max) const;

  private:
    Raft raft_;
};


}  // namespace vraft

#endif

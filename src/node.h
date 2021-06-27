#ifndef __VRAFT_NODE_H__
#define __VRAFT_NODE_H__

#include <random>
#include <functional>
#include <glog/logging.h>
#include "log.h"
#include "raft.h"
#include "status.h"
#include "common.h"
#include "nodeid.h"
#include "vraft_rpc.grpc.pb.h"

namespace vraft {

class Node {
  public:
    static Node&
    GetInstance() {
        static Node instance;
        return instance;
    }

    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    Status Init();
    Status Start();

    const NodeId &id() const {
        return id_;
    }

    void OnPing(const vraft_rpc::Ping &request, vraft_rpc::PingReply &reply);
    Status Ping(const vraft_rpc::Ping &request, const std::string &address);
    Status OnPingReply(const vraft_rpc::PingReply &reply);
    Status PingAll();
    Status PingPeers();

  private:
    void Sleep(int min, int max) const;

  private:
    Node();
    ~Node() = default;

    Raft raft_;
    NodeId id_;
};

}  // namespace vraft

#endif

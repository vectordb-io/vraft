#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include "env.h"
#include "node.h"
#include "util.h"
#include "config.h"

namespace vraft {

Node::Node()
    :id_(Config::GetInstance().me().ToString()) {
}

Status
Node::Init() {
    Env::GetInstance().grpc_server()->set_on_ping_cb(
        std::bind(&Node::OnPing, this, std::placeholders::_1, std::placeholders::_2)
    );

    auto s = raft_.Init();
    assert(s.ok());

    return Status::OK();
}

Status
Node::Start() {
    LOG(INFO) << "node start ...";
    while(!Env::GetInstance().timer()->running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (Config::GetInstance().ping()) {
        vraft::Env::GetInstance().timer()->RunEvery(std::bind(&vraft::Node::PingPeers, this), 3000);
    }

    vraft::Env::GetInstance().timer()->RunEvery(std::bind(&vraft::Raft::PrintId, &raft_), 1000);

    auto s = raft_.Start();
    assert(s.ok());

    return Status::OK();
}

void
Node::OnPing(const vraft_rpc::Ping &request, vraft_rpc::PingReply &reply) {
    NodeId node_id(request.node_id());
    LOG(INFO) << "receive from " << node_id.address() << ": " << request.msg();
    reply.set_node_id(id_.code());
    if (request.msg() == "ping") {
        reply.set_msg("pang");
    } else {
        reply.set_msg("no sounds");
    }
    LOG(INFO) << "send to " << node_id.address() << ": " << reply.msg();
}

Status
Node::Ping(const vraft_rpc::Ping &request, const std::string &address) {
    LOG(INFO) << "send to " << address << ": " << request.msg();
    auto s = Env::GetInstance().AsyncPing(
                 request,
                 address,
                 std::bind(&Node::OnPingReply, this, std::placeholders::_1)
             );
    return s;
}

Status
Node::OnPingReply(const vraft_rpc::PingReply &reply) {
    NodeId node_id(reply.node_id());
    LOG(INFO) << "receive from " << node_id.address() << ": " << reply.msg();
    return Status::OK();
}

Status
Node::PingAll() {
    for (auto &hp : Config::GetInstance().addresses()) {
        vraft_rpc::Ping request;
        request.set_node_id(id_.code());
        request.set_msg("ping");
        auto s = Ping(request, hp.ToString());
        assert(s.ok());
    }
    return Status::OK();
}

Status
Node::PingPeers() {
    for (auto &hp : Config::GetInstance().peers()) {
        vraft_rpc::Ping request;
        request.set_node_id(id_.code());
        request.set_msg("ping");
        auto s = Ping(request, hp.ToString());
        assert(s.ok());
    }
    return Status::OK();
}

void
Node::Sleep(int min, int max) const {
    int timeout_ms = util::RandomInt(min, max);
    LOG(INFO) << "sleep " << timeout_ms << " ms";
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
}

}  // namespace vraft

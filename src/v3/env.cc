#include "util.h"
#include "env.h"
#include "node.h"

namespace vraft {

Env::Env()
    :storage_(Config::GetInstance().path() + "/store") {
}

Env::~Env() {
}

Status
Env::AsyncPing(const vraft_rpc::Ping &request, const std::string &address, PingFinishCallBack cb) {
    auto s = grpc_server_.AsyncPing(request, address, cb);
    return s;
}

Status
Env::AsyncRequestVote(const vraft_rpc::RequestVote &request, const std::string &address, RequestVoteFinishCallBack cb) {
    auto s = grpc_server_.AsyncRequestVote(request, address, cb);
    return s;
}

Status
Env::AsyncAppendEntries(const vraft_rpc::AppendEntries &request, const std::string &address, AppendEntriesFinishCallBack cb) {
    auto s = grpc_server_.AsyncAppendEntries(request, address, cb);
    return s;
}

Status
Env::AsyncClientRequestReply(const vraft_rpc::ClientRequestReply &reply, void *call) {
    auto s = grpc_server_.AsyncClientRequestReply(reply, call);
    return s;
}

Status
Env::CurrentTerm(int64_t &term) const {
    auto s = storage_.CurrentTerm(term);
    return s;
}

Status
Env::PersistCurrentTerm(int64_t term) {
    auto s = storage_.PersistCurrentTerm(term);
    return s;
}

Status
Env::VoteFor(uint64_t &node_id) const {
    auto s = storage_.VoteFor(node_id);
    return s;
}

Status
Env::PersistVoteFor(uint64_t node_id) {
    auto s = storage_.PersistVoteFor(node_id);
    return s;
}

Status
Env::Init() {
    Status s;

    if (!util::DirOK(Config::GetInstance().path())) {
        util::MakeDir(Config::GetInstance().path());
    }

    s = storage_.Init();
    assert(s.ok());

    return Status::OK();
}

Status
Env::Start() {
    Status s;
    thread_pool_.Start(1);
    s = grpc_server_.Start();
    assert(s.ok());
    s = timer_.Start();
    assert(s.ok());

    return Status::OK();
}

Status
Env::Stop() {
    Status s;
    s = timer_.Stop();
    assert(s.ok());
    s = grpc_server_.Stop();
    assert(s.ok());
    thread_pool_.Stop();

    return Status::OK();
}

}  // namespace vraft

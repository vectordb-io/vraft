#include "util.h"
#include "env.h"
#include "node.h"

namespace vraft {

Env::Env()
    :log_(Config::GetInstance().path() + "/log"),
     storage_(Config::GetInstance().path() + "/store") {
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
Env::CurrentTerm(int64_t &term) const {
    auto s = storage_.CurrentTerm(term);
    return s;
}

Status
Env::CurrentTermPersist(int64_t term) {
    auto s = storage_.CurrentTermPersist(term);
    return s;
}

Status
Env::VoteFor(std::string &vote_for) const {
    auto s = storage_.VoteFor(vote_for);
    return s;
}

Status
Env::VoteForPersist(const std::string &vote_for) {
    auto s = storage_.VoteForPersist(vote_for);
    return s;
}

Status
Env::Init() {
    Status s;

    if (!util::DirOK(Config::GetInstance().path())) {
        util::Mkdir(Config::GetInstance().path());
    }

    s = storage_.Init();
    assert(s.ok());

    s = log_.Init();
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

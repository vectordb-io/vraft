#include "env.h"
#include "coding.h"
#include "util.h"
#include "node.h"
#include "raft.h"

namespace vraft {

std::string
State2String(State s) {
    if (s == STATE_LEADER) {
        return "leader";
    } else if (s == STATE_CANDIDATE) {
        return "candidate";
    } else if (s == STATE_FOLLOWER) {
        return "follower";
    } else {
        return "invalid_state";
    }
}

// ---------------------------------------------------------------------------------------------
// class RequestVoteManager
RequestVoteManager::RequestVoteManager(int quorum)
    :term_(-1),
     quorum_(quorum) {
}

RequestVoteManager::~RequestVoteManager() {
}

void
RequestVoteManager::Vote(const vraft_rpc::RequestVoteReply &reply) {
    assert(reply.vote_granted());
    assert(reply.term() == term_);
    votes_.insert(std::pair<uint64_t, vraft_rpc::RequestVoteReply>(reply.node_id(), reply));
}

bool
RequestVoteManager::Majority() const {
    return static_cast<int>(votes_.size()) >= quorum_;
}

void
RequestVoteManager::Reset(int64_t term) {
    term_ = term;
    votes_.clear();
}

std::string
RequestVoteManager::ToString() const {
    jsonxx::json64 j, jret;
    j["term"] = term_;
    j["quorum"] = quorum_;
    j["votes"] = votes_.size();
//    for (auto &kv in votes_) {
//        j["votes"][kv.first] = ToString(kv.second);
//    }
    jret["RequestVoteManager"] = j;
    //return jret.dump(4, ' ');
    return jret.dump();
}

// ---------------------------------------------------------------------------------------------
// class Raft
Raft::Raft()
    :state_(STATE_INVALID),
     current_term_(-1),
     vote_for_(0),
     leader_(0),
     follower2candidate_timer_(-1),
     election_timer_(-1),
     heartbeat_timer_(-1),
     request_vote_manager_(Config::GetInstance().Quorum()) {
}

Raft::~Raft() {
}

Status
Raft::Init() {
    state_ = STATE_FOLLOWER;

    auto s = CurrentTerm(current_term_);
    if (s.IsNotFound()) {
        current_term_ = 0;
        auto s1 = PersistCurrentTerm(current_term_);
        assert(s1.ok());
    } else {
        if (!s.ok()) {
            LOG(INFO) << s.ToString();
            assert(0);
        }
    }

    s = VoteFor(vote_for_);
    if (s.IsNotFound()) {
        vote_for_ = 0;
        auto s1 = PersistVoteFor(vote_for_);
        assert(s1.ok());
    } else {
        if (!s.ok()) {
            LOG(INFO) << s.ToString();
            assert(0);
        }
    }

    Env::GetInstance().grpc_server()->set_on_client_request_cb(
        std::bind(&Raft::OnClientRequest, this, std::placeholders::_1, std::placeholders::_2)
    );

    Env::GetInstance().grpc_server()->set_on_request_vote_cb(
        std::bind(&Raft::OnRequestVote, this, std::placeholders::_1, std::placeholders::_2)
    );

    Env::GetInstance().grpc_server()->set_on_append_entries_cb(
        std::bind(&Raft::OnAppendEntries, this, std::placeholders::_1, std::placeholders::_2)
    );

    return Status::OK();
}

Status
Raft::Start() {
    LOG(INFO) << "raft start ...";
    BeFollower();
    return Status::OK();
}

void
Raft::OnClientRequest(const vraft_rpc::ClientRequest &request, void *async_flag) {

}

void
Raft::OnRequestVote(const vraft_rpc::RequestVote &request, vraft_rpc::RequestVoteReply &reply) {
    NodeId node_id(request.node_id());
    TraceOnRequestVote(request, node_id.address());

    if (request.term() > current_term_) {
        UpdateTerm(request.term());
    }
    assert(request.term() <= current_term_);

    bool log_ok, grant;
    log_ok = (request.last_log_term() > Env::GetInstance().log()->LastLogId().term()) ||
             ((request.last_log_term() == Env::GetInstance().log()->LastLogId().term()) &&
              request.last_log_index() >= Env::GetInstance().log()->LastLogId().index());
    grant = request.term() == current_term_ && log_ok && (vote_for_ == 0 || vote_for_ == request.node_id());
    if (grant) {
        vote_for_ = request.node_id();
        auto s = PersistVoteFor(vote_for_);
        assert(s.ok());
    }

    if (state_ == STATE_FOLLOWER) {
        ResetElectionTimer();
    }

    reply.set_term(current_term_);
    reply.set_vote_granted(grant);
    reply.set_node_id(Node::GetInstance().id().code());

    TraceRequestVoteReply(reply, node_id.address());
}

Status
Raft::RequestVote(const vraft_rpc::RequestVote &request, const std::string &address) {
    TraceRequestVote(request, address);
    auto s = Env::GetInstance().AsyncRequestVote(
                 request,
                 address,
                 std::bind(&Raft::OnRequestVoteReply, this, std::placeholders::_1)
             );
    if (!s.ok()) {
        LOG(INFO) << s.ToString();
    }
    return Status::OK();
}

Status
Raft::OnRequestVoteReply(const vraft_rpc::RequestVoteReply &reply) {
    NodeId node_id(reply.node_id());
    TraceOnRequestVoteReply(reply, node_id.address());

    if (reply.term() > current_term_) {
        UpdateTerm(reply.term());

    } else if (reply.term() == current_term_) {
        if (reply.vote_granted()) {
            request_vote_manager_.Vote(reply);
            if (request_vote_manager_.Majority()) {
                Candidate2Leader();
            }
        }
    } else {
        LOG(INFO) << "old term, ignore.";
    }
    return Status::OK();
}

Status
Raft::RequestVotePeers(int64_t term) {
    TraceLog("RequestVotePeers", __func__);

    vraft_rpc::RequestVote request;
    request.set_term(term);
    request.set_node_id(Node::GetInstance().id().code());
    request.set_last_log_index(Env::GetInstance().log()->LastLogId().index());
    request.set_last_log_term(Env::GetInstance().log()->LastLogId().term());

    for (auto &hp : Config::GetInstance().peers()) {
        auto s = RequestVote(request, hp.ToString());
        if (!s.ok()) {
            LOG(INFO) << s.ToString();
            assert(0);
        }
    }

    return Status::OK();
}

Status
Raft::RequestVotePeers() {
    RequestVotePeers(current_term_);
    return Status::OK();
}

void
Raft::OnAppendEntries(const vraft_rpc::AppendEntries &request, vraft_rpc::AppendEntriesReply &reply) {
    NodeId node_id(request.node_id());
    TraceOnAppendEntries(request, node_id.address());

    bool success;
    if (request.term() > current_term_) {
        UpdateTerm(request.term());
        success = true;
    } else if (request.term() == current_term_) {
        success = true;
    } else {
        success = false;
    }

    if (success == true) {
        leader_ = request.node_id();
    }

    if (state_ == STATE_FOLLOWER) {
        ResetF2CTimer();

    } else if (state_ == STATE_CANDIDATE) {
        BeFollower();

    } else if (state_ == STATE_LEADER) {

    }

    reply.set_term(current_term_);
    reply.set_success(success);
    reply.set_node_id(Node::GetInstance().id().code());

    TraceAppendEntriesReply(reply, node_id.address());
}

Status
Raft::AppendEntries(const vraft_rpc::AppendEntries &request, const std::string &address) {
    TraceAppendEntries(request, address);

    auto s = Env::GetInstance().AsyncAppendEntries(
                 request,
                 address,
                 std::bind(&Raft::OnAppendEntriesReply, this, std::placeholders::_1)
             );
    return s;
}

Status
Raft::OnAppendEntriesReply(const vraft_rpc::AppendEntriesReply &reply) {
    NodeId node_id(reply.node_id());
    TraceOnAppendEntriesReply(reply, node_id.address());

    if (reply.term() > current_term_) {
        UpdateTerm(reply.term());
    } else if (reply.term() == current_term_) {

    } else {
        LOG(INFO) << "old term, ignore.";
    }
    return Status::OK();
}

Status
Raft::AppendEntriesPeers() {
    vraft_rpc::AppendEntries request;
    request.set_term(current_term_);
    request.set_node_id(Node::GetInstance().id().code());
    // set others

    for (auto &hp : Config::GetInstance().peers()) {
        auto s = AppendEntries(request, hp.ToString());
        assert(s.ok());
    }

    return Status::OK();
}

void Raft::PrintId() const {
    LOG(INFO) << Node::GetInstance().id().ToString() << " : I am " << State2String(state_);
    TraceLog("PrintId", __func__);
}

void
Raft::BeFollower() {
    TraceLog("BeFollower", __func__);

    state_ = STATE_FOLLOWER;
    leader_ = 0;

    ClearElectionTimer();
    ClearHeartbeatTimer();
    ResetF2CTimer();
}

void
Raft::Elect() {
    TraceLog("Elect", __func__);

    if (state_ == STATE_CANDIDATE) {
        NextTerm();
        ClearVoteFor();
        request_vote_manager_.Reset(current_term_);
        VoteForSelf();
        RequestVotePeers();
        ResetElectionTimer();
    }
}

void
Raft::VoteForTerm(int64_t term, uint64_t node_id) {
    assert(term == current_term_);
    assert(vote_for_ == 0);
    vote_for_ = node_id;
    auto s = PersistVoteFor(vote_for_);
    assert(s.ok());
}

void
Raft::VoteForSelf() {
    VoteForTerm(current_term_, Node::GetInstance().id().code());
    vraft_rpc::RequestVoteReply reply;
    reply.set_term(current_term_);
    reply.set_vote_granted(true);
    reply.set_node_id(Node::GetInstance().id().code());
    request_vote_manager_.Vote(reply);
}

bool
Raft::HasVoted(int64_t term) const {
    return !(vote_for_ == 0);
}

void
Raft::NextTerm() {
    current_term_++;
    auto s = PersistCurrentTerm(current_term_);
    assert(s.ok());
}

Status
Raft::ClearVoteFor() {
    vote_for_ = 0;
    auto s = PersistVoteFor(vote_for_);
    assert(s.ok());
    return Status::OK();
}

void
Raft::UpdateTerm(int64_t term) {
    TraceLog("UpdateTerm", __func__);

    if (term > current_term_) {
        current_term_ = term;
        auto s = PersistCurrentTerm(current_term_);
        assert(s.ok());

        BeFollower();

        s = ClearVoteFor();
        assert(s.ok());
    }
}

void
Raft::ResetF2CTimer() {
    int timer_ms = util::random_int(Config::GetInstance().election_timeout(),
                                    2 * Config::GetInstance().election_timeout());
    if (-1 == follower2candidate_timer_) {
        follower2candidate_timer_ = Env::GetInstance().timer()->RunAfter(
                                        std::bind(&Raft::EqFollower2Candidate, this), timer_ms);
        assert(follower2candidate_timer_ != -1);
    } else {
        auto s = Env::GetInstance().timer()->ResetRunAfter(follower2candidate_timer_, timer_ms);
        assert(s.ok());
    }
}

void
Raft::ClearF2CTimer() {
    if (-1 != follower2candidate_timer_) {
        Env::GetInstance().timer()->Stop(follower2candidate_timer_);
    }
}

void
Raft::EqFollower2Candidate() {
    Env::GetInstance().thread_pool()->ProduceOne(std::bind(&Raft::Follower2Candidate, this));
}

void
Raft::ResetElectionTimer() {
    int timer_ms = util::random_int(Config::GetInstance().election_timeout(),
                                    2 * Config::GetInstance().election_timeout());
    if (-1 == election_timer_) {
        election_timer_= Env::GetInstance().timer()->RunAfter(
                             std::bind(&Raft::EqElect, this), timer_ms);
        if (election_timer_!= -1) {
            char buf[128];
            snprintf(buf, sizeof(buf), "election_timer_:%d, timer_ms:%d", election_timer_, timer_ms);
            LOG(INFO) << buf;
        }
    } else {
        auto s = Env::GetInstance().timer()->ResetRunAfter(election_timer_, timer_ms);
        if (!s.ok()) {
            LOG(INFO) << s.ToString();
            assert(0);
        }
    }
}

void
Raft::ClearElectionTimer() {
    if (-1 != election_timer_) {
        Env::GetInstance().timer()->Stop(election_timer_);
    }
}

void
Raft::EqElect() {
    Env::GetInstance().thread_pool()->ProduceOne(std::bind(&Raft::Elect, this));
}

void
Raft::ResetHeartbeatTimer() {
    int timer_ms = Config::GetInstance().heartbeat_timeout();
    if (-1 == heartbeat_timer_) {
        heartbeat_timer_ = Env::GetInstance().timer()->RunEvery(
                               std::bind(&Raft::EqAppendEntriesPeers, this), timer_ms);
        assert(heartbeat_timer_ != -1);
    } else {
        auto s = Env::GetInstance().timer()->ResetRunEvery(heartbeat_timer_, timer_ms);
        assert(s.ok());
    }
}

void
Raft::EqAppendEntriesPeers() {
    Env::GetInstance().thread_pool()->ProduceOne(std::bind(&Raft::AppendEntriesPeers, this));
}

void
Raft::ClearHeartbeatTimer() {
    if (-1 != heartbeat_timer_) {
        Env::GetInstance().timer()->Stop(heartbeat_timer_);
    }
}

void
Raft::Follower2Candidate() {
    TraceLog("Follower2Candidate", __func__);

    assert(state_ == STATE_FOLLOWER);
    state_ = STATE_CANDIDATE;
    ClearF2CTimer();
    ClearHeartbeatTimer();
    Elect();
}

void
Raft::Candidate2Leader() {
    TraceLog("Candidate2Leader", __func__);

    if (state_ == STATE_CANDIDATE) {
        state_ = STATE_LEADER;
        leader_ = Node::GetInstance().id().code();
        ClearF2CTimer();
        ClearElectionTimer();
        ResetHeartbeatTimer();
        auto s = AppendEntriesPeers();
        assert(s.ok());
    }
}

void
Raft::Leader2Follower() {
    TraceLog("Leader3Follower", __func__);

    assert(state_ == STATE_LEADER);
    BeFollower();
}

void
Raft::Candidate2Follower() {
    TraceLog("Candidate2Follower", __func__);

    assert(state_ == STATE_CANDIDATE);
    BeFollower();
}

Status
Raft::CurrentTerm(int64_t &term) const {
    auto s = Env::GetInstance().CurrentTerm(term);
    return s;
}

Status
Raft::PersistCurrentTerm(int64_t term) {
    auto s = Env::GetInstance().PersistCurrentTerm(term);
    return s;
}

Status
Raft::VoteFor(uint64_t &node_id) const {
    auto s = Env::GetInstance().VoteFor(node_id);
    return s;
}

Status
Raft::PersistVoteFor(uint64_t node_id) {
    auto s = Env::GetInstance().PersistVoteFor(node_id);
    return s;
}

// for debug -------------
void
Raft::TraceRequestVote(const vraft_rpc::RequestVote &msg, const std::string &address) const {
    std::string log_str = Node::GetInstance().id().address();
    log_str.append(" : send to ").append(address).append(" ").append(::vraft::ToString(msg)).append("\n");
    log_str.append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;
}

void
Raft::TraceOnRequestVote(const vraft_rpc::RequestVote &msg, const std::string &address) const {
    std::string log_str = Node::GetInstance().id().address();
    log_str.append(" : recv from ").append(address).append(" ").append(::vraft::ToString(msg)).append("\n");
    log_str.append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;
}

void
Raft::TraceRequestVoteReply(const vraft_rpc::RequestVoteReply &msg, const std::string &address) const {
    std::string log_str = Node::GetInstance().id().address();
    log_str.append(" : send to ").append(address).append(" ").append(::vraft::ToString(msg)).append("\n");
    log_str.append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;
}

void
Raft::TraceOnRequestVoteReply(const vraft_rpc::RequestVoteReply &msg, const std::string &address) const {
    std::string log_str = Node::GetInstance().id().address();
    log_str.append(" : recv from ").append(address).append(" ").append(::vraft::ToString(msg)).append("\n");
    log_str.append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;
}

void
Raft::TraceAppendEntries(const vraft_rpc::AppendEntries &msg, const std::string &address) const {
    std::string log_str = Node::GetInstance().id().address();
    log_str.append(" : send to ").append(address).append(" ").append(::vraft::ToString(msg)).append("\n");
    log_str.append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;
}

void
Raft::TraceOnAppendEntries(const vraft_rpc::AppendEntries &msg, const std::string &address) const {
    std::string log_str = Node::GetInstance().id().address();
    log_str.append(" : recv from ").append(address).append(" ").append(::vraft::ToString(msg)).append("\n");
    log_str.append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;
}

void
Raft::TraceAppendEntriesReply(const vraft_rpc::AppendEntriesReply &msg, const std::string &address) const {
    std::string log_str = Node::GetInstance().id().address();
    log_str.append(" : send to ").append(address).append(" ").append(::vraft::ToString(msg)).append("\n");
    log_str.append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;

}

void
Raft::TraceOnAppendEntriesReply(const vraft_rpc::AppendEntriesReply &msg, const std::string &address) const {
    std::string log_str = Node::GetInstance().id().address();
    log_str.append(" : recv from ").append(address).append(" ").append(::vraft::ToString(msg)).append("\n");
    log_str.append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;
}

void
Raft::TraceLog(const std::string &log_flag, const std::string func_name) const {
    std::string log_str = "debug ";
    log_str.append("--").append(log_flag).append("-- [func:").append(func_name).append("]\n").append(ToStringPretty()).append("\n\n");
    LOG(INFO) << log_str;
}

jsonxx::json64
Raft::ToJson() const {
    jsonxx::json64 j, jret;
    j["state"] = State2String(state_);
    j["current_term"] = current_term_;
    NodeId nid_vote_for(vote_for_);
    j["vote_for"] = nid_vote_for.ToString();
    NodeId nid_leader(leader_);
    j["leader"] = nid_leader.ToString();
    j["request_vote_manager"] = request_vote_manager_.ToString();

    jret["Raft"] = j;
    return jret;
}

std::string
Raft::ToString() const {
    return ToJson().dump();
}

std::string
Raft::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}


}  // namespace vraft

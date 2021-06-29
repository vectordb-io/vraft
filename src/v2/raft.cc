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
// class VotesGranted
VotesGranted::VotesGranted(int quorum)
    :term_(0),
     quorum_(quorum) {
}

VotesGranted::~VotesGranted() {
}

void
VotesGranted::Vote(const vraft_rpc::RequestVoteReply &reply) {
    assert(reply.vote_granted());
    assert(reply.term() == term_);
    votes_.insert(std::pair<uint64_t, vraft_rpc::RequestVoteReply>(reply.node_id(), reply));
}

bool
VotesGranted::Majority() const {
    return static_cast<int>(votes_.size()) >= quorum_;
}

void
VotesGranted::Reset(int64_t term) {
    term_ = term;
    votes_.clear();
}

jsonxx::json64
VotesGranted::ToJson() const {
    jsonxx::json64 j, jret;
    j["term"] = term_;
    j["quorum"] = quorum_;
    j["votes"] = votes_.size();
    jret["VotesGranted"] = j;
    return jret;
}

std::string
VotesGranted::ToString() const {
    return ToJson().dump();
}

std::string
VotesGranted::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}

// ---------------------------------------------------------------------------------------------
// class VotesGranted
VotesResponded::VotesResponded()
    :term_(0) {
    for (auto &hp : Config::GetInstance().addresses()) {
        NodeId nid(hp.ToString());
        all_nodes_.insert(nid.code());
    }

    unresponded_ = all_nodes_;
}

bool
VotesResponded::IsResponded(int64_t node_id) const {
    auto it = responded_.find(node_id);
    return it != responded_.end();
}

void
VotesResponded::Add(const vraft_rpc::RequestVoteReply &reply) {
    assert(reply.term() == term_);
    auto it = unresponded_.find(reply.node_id());
    assert(it != unresponded_.end());

    responded_.insert(std::pair<uint64_t, vraft_rpc::RequestVoteReply>(reply.node_id(), reply));
    unresponded_.erase(it);
}

void
VotesResponded::Reset(int64_t term) {
    term_ = term;
    responded_.clear();
    unresponded_ = all_nodes_;
}

jsonxx::json64
VotesResponded::ToJson() const {
    jsonxx::json64 j, jret;
    j["term"] = term_;
    j["all_nodes"] = all_nodes_.size();
    j["responded"] = responded_.size();
    j["unresponded"] = unresponded_.size();
    jret["VotesResponded"] = j;
    return jret;
}

std::string
VotesResponded::ToString() const {
    return ToJson().dump();
}

std::string
VotesResponded::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}

// ---------------------------------------------------------------------------------------------
// class PersistedTerm

PersistedTerm::PersistedTerm()
    :term_(0) {
}

Status
PersistedTerm::Init() {
    auto s = Env::GetInstance().CurrentTerm(term_);
    if (s.IsNotFound()) {
        term_ = 0;
        auto s1 = Env::GetInstance().PersistCurrentTerm(term_);
        assert(s1.ok());
    } else {
        if (!s.ok()) {
            LOG(INFO) << s.ToString();
            assert(0);
        }
    }
    return Status::OK();
}

void
PersistedTerm::Next() {
    term_++;
    auto s = Env::GetInstance().PersistCurrentTerm(term_);
    assert(s.ok());
}

int64_t
PersistedTerm::get() const {
    return term_;
}

void
PersistedTerm::set(int64_t term) {
    term_ = term;
    auto s = Env::GetInstance().PersistCurrentTerm(term_);
    assert(s.ok());
}

// ---------------------------------------------------------------------------------------------
// class PersistedVoteFor

PersistedVoteFor::PersistedVoteFor()
    :node_id_(0) {
}

Status
PersistedVoteFor::Init() {
    auto s = Env::GetInstance().VoteFor(node_id_);
    if (s.IsNotFound()) {
        node_id_ = 0;
        auto s1 = Env::GetInstance().PersistVoteFor(node_id_);
        assert(s1.ok());
    } else {
        if (!s.ok()) {
            LOG(INFO) << s.ToString();
            assert(0);
        }
    }
    return Status::OK();
}

bool
PersistedVoteFor::HasVoted() const {
    return node_id_ != 0;
}

void
PersistedVoteFor::Vote(uint64_t node_id) {
    assert(!HasVoted());
    assert(node_id != 0);
    node_id_ = node_id;
    auto s = Env::GetInstance().PersistVoteFor(node_id_);
    assert(s.ok());
}

void
PersistedVoteFor::Clear() {
    node_id_ = 0;
    auto s = Env::GetInstance().PersistVoteFor(node_id_);
    assert(s.ok());
}

NodeId
PersistedVoteFor::ToNodeId() const {
    NodeId nid(node_id_);
    return nid;
}

uint64_t
PersistedVoteFor::ToUInt() const {
    return node_id_;
}

// ---------------------------------------------------------------------------------------------
// class ServerVars
Status
ServerVars::Init() {
    auto s = current_term_.Init();
    assert(s.ok());

    s = vote_for_.Init();
    assert(s.ok());

    return Status::OK();
}

jsonxx::json64
ServerVars::ToJson() const {
    jsonxx::json64 j, jret;
    j["state"] = State2String(state_);
    j["current_term"] = current_term_.get();
    j["vote_for"] = vote_for_.ToNodeId().ToJsonTiny();
    jret["ServerVars"] = j;
    return jret;
}

std::string
ServerVars::ToString() const {
    return ToJson().dump();
}

std::string
ServerVars::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}

// ---------------------------------------------------------------------------------------------
// class CandidateVars
CandidateVars::CandidateVars(int quorum)
    :votes_granted_(quorum) {
}

Status
CandidateVars::Init() {
    return Status::OK();
}

jsonxx::json64
CandidateVars::ToJson() const {
    jsonxx::json64 j, jret;
    j["votes_granted"] = votes_granted_.ToJson();
    j["votes_responded"] = votes_responded_.ToJson();
    jret["CandidateVars"] = j;
    return jret;
}

std::string
CandidateVars::ToString() const {
    return ToJson().dump();
}

std::string
CandidateVars::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}

// ---------------------------------------------------------------------------------------------
// class LeaderVars
Status
LeaderVars::Init() {
    for (auto &hp : Config::GetInstance().addresses()) {
        NodeId nid(hp.ToString());
        next_index_.insert(std::pair<uint64_t, int>(nid.code(), 1));
        match_index_.insert(std::pair<uint64_t, int>(nid.code(), 0));
    }

    return Status::OK();
}

jsonxx::json64
LeaderVars::ToJson() const {
    jsonxx::json64 j, jret;
    for (auto &kv : next_index_) {
        NodeId nid(kv.first);
        int index = kv.second;
        j["next_index"][nid.address()] = index;
    }

    for (auto &kv : match_index_) {
        NodeId nid(kv.first);
        int index = kv.second;
        j["match_index"][nid.address()] = index;
    }

    jret["LeaderVars"] = j;
    return jret;
}

std::string
LeaderVars::ToString() const {
    return ToJson().dump();
}

std::string
LeaderVars::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}

// ---------------------------------------------------------------------------------------------
// class LogVars
LogVars::LogVars(const std::string &log_path)
    :commit_index_(0),
     log_(log_path) {
}

Status
LogVars::Init() {
    auto s = log_.Init();
    assert(s.ok());
    return Status::OK();
}

jsonxx::json64
LogVars::ToJson() const {
    jsonxx::json64 j, jret;
    j["commit_index"] = commit_index_;
    j["log"] = log_.ToJson();
    jret["LogVars"] = j;
    return jret;
}

std::string
LogVars::ToString() const {
    return ToJson().dump();
}

std::string
LogVars::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}

// ---------------------------------------------------------------------------------------------
// class Raft
Raft::Raft()
    :candidate_vars_(Config::GetInstance().Quorum()),
     log_vars_(Config::GetInstance().path() + "/log"),
     leader_(0),
     follower2candidate_timer_(-1),
     election_timer_(-1),
     heartbeat_timer_(-1) {
}

Raft::~Raft() {
}

Status
Raft::Init() {
    //state_ = STATE_FOLLOWER;

    auto s = server_vars_.Init();
    assert(s.ok());

    s = candidate_vars_.Init();
    assert(s.ok());

    s = leader_vars_.Init();
    assert(s.ok());

    s = log_vars_.Init();
    assert(s.ok());

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

    return Status::OK();
}

Status
Raft::RequestVotePeers() {
    TraceLog("RequestVotePeers", __func__);



    return Status::OK();
}

void
Raft::OnAppendEntries(const vraft_rpc::AppendEntries &request, vraft_rpc::AppendEntriesReply &reply) {
    NodeId node_id(request.node_id());
    TraceOnAppendEntries(request, node_id.address());

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

    return Status::OK();
}

Status
Raft::AppendEntriesPeers() {
    TraceLog("AppendEntriesPeers", __func__);


    return Status::OK();
}

void Raft::PrintId() const {
    LOG(INFO) << Node::GetInstance().id().ToString() << " : I am " << State2String(server_vars_.state());
    TraceLog("PrintId", __func__);
}

void
Raft::BeFollower() {
    TraceLog("BeFollower", __func__);

    //state_ = STATE_FOLLOWER;
    //leader_ = 0;

    //ClearElectionTimer();
    //ClearHeartbeatTimer();
    //ResetF2CTimer();
}

void
Raft::Elect() {
    TraceLog("Elect", __func__);

    //if (state_ == STATE_CANDIDATE) {
    //    NextTerm();
    //    ClearVoteFor();
    //    request_vote_manager_.Reset(current_term_);
    //    VoteForSelf();
    //    RequestVotePeers();
    //    ResetElectionTimer();
    //}
}

void
Raft::VoteForTerm(int64_t term, uint64_t node_id) {
    //assert(term == current_term_);
    //assert(vote_for_ == 0);
    //vote_for_ = node_id;
    //auto s = PersistVoteFor(vote_for_);
    //assert(s.ok());
}

void
Raft::VoteForSelf() {
    //VoteForTerm(current_term_, Node::GetInstance().id().code());
    //vraft_rpc::RequestVoteReply reply;
    //reply.set_term(current_term_);
    //reply.set_vote_granted(true);
    //reply.set_node_id(Node::GetInstance().id().code());
    //request_vote_manager_.Vote(reply);
}

void
Raft::UpdateTerm(int64_t term) {
    TraceLog("UpdateTerm", __func__);

    //if (term > current_term_) {
    //    current_term_ = term;
    //    auto s = PersistCurrentTerm(current_term_);
    //    assert(s.ok());

    //    BeFollower();

    //    s = ClearVoteFor();
    //    assert(s.ok());
    //}
}

void
Raft::Follower2Candidate() {
    TraceLog("Follower2Candidate", __func__);

    //assert(state_ == STATE_FOLLOWER);
    //state_ = STATE_CANDIDATE;
    //ClearF2CTimer();
    //ClearHeartbeatTimer();
    //Elect();
}

void
Raft::Candidate2Leader() {
    TraceLog("Candidate2Leader", __func__);

    //if (state_ == STATE_CANDIDATE) {
    //    state_ = STATE_LEADER;
    //    leader_ = Node::GetInstance().id().code();
    //    ClearF2CTimer();
    //    ClearElectionTimer();
    //    ResetHeartbeatTimer();
    //    auto s = AppendEntriesPeers();
    //    assert(s.ok());
    //}
}

void
Raft::Leader2Follower() {
    TraceLog("Leader3Follower", __func__);

    //assert(state_ == STATE_LEADER);
    //BeFollower();
}

void
Raft::Candidate2Follower() {
    TraceLog("Candidate2Follower", __func__);

    //assert(state_ == STATE_CANDIDATE);
    //BeFollower();
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
    j["server_vars"] = server_vars_.ToJson();
    j["candidate_vars"] = candidate_vars_.ToJson();
    j["leader_vars"] = leader_vars_.ToJson();
    j["log_vars"] = log_vars_.ToJson();

    NodeId nid(leader_);
    j["leader"] = nid.ToJson();

    //j["state"] = State2String(server_vars_.state());
    //j["current_term"] = server_vars_.current_term();
    //NodeId nid_vote_for(server_vars_.current_term().ToUInt());
    //j["vote_for"] = nid_vote_for.ToString();
    //NodeId nid_leader(leader_);
    //j["leader"] = nid_leader.ToString();
    //j["request_vote_manager"] = request_vote_manager_.ToString();

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

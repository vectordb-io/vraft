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
     quorum_(quorum),
     to_leader_(false) {
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
    to_leader_ = false;
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
        term_ = 1;
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
    state_ = STATE_FOLLOWER;

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
    votes_granted_.Reset(Node::GetInstance().raft().CurrentTerm());
    votes_responded_.Reset(Node::GetInstance().raft().CurrentTerm());

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
    for (auto &hp : Config::GetInstance().peers()) {
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
     election_timer_(-1),
     election_random_ms_(0),
     heartbeat_timer_(-1),
     heartbeat_random_ms_(0) {
}

Raft::~Raft() {
}

Status
Raft::Init() {
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

    if (request.term() > CurrentTerm()) {
        UpdateTerm(request.term());
    }
    assert(request.term() <= CurrentTerm());

    bool log_ok = (request.last_log_term() > log_vars_.log().LastLogTerm()) ||
                  ((request.last_log_term() == log_vars_.log().LastLogTerm()) &&
                   (request.last_log_index() >= log_vars_.log().Len()));
    bool grant = (request.term() == CurrentTerm()) && log_ok &&
                 (!server_vars_.vote_for().HasVoted() ||
                  server_vars_.vote_for().ToUInt() == request.node_id());

    if (grant) {
        server_vars_.mutable_vote_for().Vote(request.node_id());
    }

    reply.set_term(CurrentTerm());
    reply.set_vote_granted(grant);
    reply.set_node_id(Node::GetInstance().id().code());

    TraceRequestVoteReply(reply, node_id.address());
}

Status
Raft::RequestVote(const vraft_rpc::RequestVote &request, const std::string &address) {
    TraceRequestVote(request, address);
    assert(server_vars_.state() == STATE_CANDIDATE);

    auto s = Env::GetInstance().AsyncRequestVote(
                 request,
                 address,
                 std::bind(&Raft::OnRequestVoteReply, this, std::placeholders::_1)
             );
    return s;
}

Status
Raft::OnRequestVoteReply(const vraft_rpc::RequestVoteReply &reply) {
    NodeId node_id(reply.node_id());
    TraceOnRequestVoteReply(reply, node_id.address());

    if (reply.term() < CurrentTerm()) {
        char buf[256];
        snprintf(buf, sizeof(buf), "DropStaleResponse, receive term:%ld, current term:%ld", reply.term(), CurrentTerm());
        std::string log_str(buf);
        LOG(INFO) << log_str;
        return Status::OK();
    }

    if (reply.term() > CurrentTerm()) {
        UpdateTerm(reply.term());
    }
    assert(reply.term() == CurrentTerm());

    candidate_vars_.mutable_votes_responded().Add(reply);
    if (reply.vote_granted()) {
        candidate_vars_.mutable_votes_granted().Vote(reply);
        if (candidate_vars_.mutable_votes_granted().Majority()) {
            if (!candidate_vars_.mutable_votes_granted().to_leader()) {
                Candidate2Leader();
                candidate_vars_.mutable_votes_granted().set_to_leader();
            }
        }
    }

    return Status::OK();
}


void
Raft::OnAppendEntries(const vraft_rpc::AppendEntries &request, vraft_rpc::AppendEntriesReply &reply) {
    NodeId node_id(request.node_id());
    TraceOnAppendEntries(request, node_id.address());

    if (request.term() > CurrentTerm()) {
        UpdateTerm(request.term());
    }
    assert(request.term() <= CurrentTerm());

    if (request.entries_size() > 0) {
        assert(request.entries_size() == 1);
    }

    int local_prev_log_term;
    if (request.prev_log_index() > 0 &&
            request.prev_log_index() <= log_vars_.log().Len()) {
        Entry entry;
        auto s = log_vars_.log().GetEntry(request.prev_log_index(), entry);
        assert(s.ok());
        local_prev_log_term = entry.term();
    }
    bool log_ok = (request.prev_log_index() == 0) ||
                  ((request.prev_log_index() > 0) &&
                   (request.prev_log_index() <= log_vars_.log().Len()) &&
                   (request.prev_log_term() == local_prev_log_term));

    // reject request
    if ((request.term() < CurrentTerm()) ||
            ((request.term() == CurrentTerm()) &&
             (CurrentState() == STATE_FOLLOWER) &&
             !log_ok)) {
        reply.set_term(CurrentTerm());
        reply.set_success(false);
        reply.set_match_index(0);
        reply.set_node_id(Node::GetInstance().id().code());
        return;
    }

    // return to follower state
    if (request.term() == CurrentTerm() &&
            CurrentState() == STATE_CANDIDATE) {
        BeFollower();
    }

    // accept request
    if (request.term() == CurrentTerm() &&
            CurrentState() == STATE_FOLLOWER &&
            log_ok) {

        leader_ = request.node_id();

        int index = request.prev_log_index() + 1;
        Entry tmp_entry;
        bool tmp_flag;

        //conflict: remove 1 entry
        tmp_flag = request.entries_size() > 0 &&
                   log_vars_.log().Len() >= request.prev_log_index();
        if (tmp_flag) {
            auto s = log_vars_.log().GetEntry(index, tmp_entry);
            assert(s.ok());
        }

        if (tmp_flag &&
                tmp_entry.term() != request.entries(0).term()) {
            int from_index = log_vars_.log().Len();
            auto s = log_vars_.mutable_log().TruncateEntries(from_index);
            assert(s.ok());
        }

        // no conflict: append entry
        if (request.entries_size() > 0 &&
                log_vars_.log().Len() == request.prev_log_index()) {
            Entry append_entry;
            Pb2Entry(request.entries(0), append_entry);
            auto s = log_vars_.mutable_log().AppendEntry(append_entry);
            assert(s.ok());
        }

        // already done with request
        tmp_flag = request.entries_size() > 0 &&
                   log_vars_.log().Len() >= request.prev_log_index();
        if (tmp_flag) {
            auto s = log_vars_.log().GetEntry(index, tmp_entry);
            assert(s.ok());
        }

        if (request.entries_size() == 0 ||
                (tmp_flag && tmp_entry.term() == request.entries(0).term())) {
            log_vars_.set_commit_index(request.commit_index());
            AdvanceCommitIndex();
            reply.set_term(CurrentTerm());
            reply.set_success(true);
            reply.set_match_index(request.prev_log_index() + request.entries_size());
            reply.set_node_id(Node::GetInstance().id().code());
        }
    }

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

    if (reply.term() < CurrentTerm()) {
        char buf[256];
        snprintf(buf, sizeof(buf), "DropStaleResponse, receive term:%ld, current term:%ld", reply.term(), CurrentTerm());
        std::string log_str(buf);
        LOG(INFO) << log_str;
        return Status::OK();
    }

    if (reply.term() > CurrentTerm()) {
        UpdateTerm(reply.term());
    }
    assert(reply.term() == CurrentTerm());

    NodeId nid(reply.node_id());
    if (reply.success()) {
        auto it1 = leader_vars_.mutable_next_index().find(nid.code());
        assert(it1 != leader_vars_.mutable_next_index().end());
        it1->second = reply.match_index() + 1;

        auto it2 = leader_vars_.mutable_match_index().find(nid.code());
        assert(it2 != leader_vars_.mutable_match_index().end());
        it2->second = reply.match_index();
    } else {
        auto it1 = leader_vars_.mutable_next_index().find(nid.code());
        assert(it1 != leader_vars_.mutable_next_index().end());
        it1->second = std::max(it1->second - 1, 1);
    }

    return Status::OK();
}

Status
Raft::RequestVotePeers() {
    TraceLog("RequestVotePeers", __func__);
    assert(server_vars_.state() == STATE_CANDIDATE);

    vraft_rpc::RequestVote request;
    request.set_term(CurrentTerm());
    request.set_node_id(Node::GetInstance().id().code());
    request.set_last_log_index(Node::GetInstance().raft().log_vars().log().LastLogIndex());
    request.set_last_log_term(Node::GetInstance().raft().log_vars().log().LastLogTerm());

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
Raft::AppendEntriesPeers() {
    TraceLog("AppendEntriesPeers", __func__);
    assert(server_vars_.state() == STATE_LEADER);

    vraft_rpc::AppendEntries request;
    request.set_term(CurrentTerm());
    request.set_node_id(Node::GetInstance().id().code());
    for (auto &hp : Config::GetInstance().peers()) {
        NodeId nid(hp.ToString());

        int next_index;
        auto it = leader_vars_.next_index().find(nid.code());
        assert(it != leader_vars_.next_index().end());
        next_index = it->second;

        int prev_log_index = next_index - 1;
        int64_t prev_log_term = 0;
        if (prev_log_index > 0) {
            Entry entry;
            auto s = log_vars_.log().GetEntry(prev_log_index, entry);
            assert(s.ok());
            prev_log_term = entry.term();
        }
        int last_entry_index = std::min(log_vars_.log().Len(), next_index);

        if (next_index <= log_vars_.log().Len()) {
            Entry send_entry;
            auto s = log_vars_.log().GetEntry(prev_log_index, send_entry);
            assert(s.ok());
            vraft_rpc::Entry *pentry = request.add_entries();
            pentry->set_term(send_entry.term());
            pentry->set_cmd(send_entry.cmd());
        }
        request.set_prev_log_index(prev_log_index);
        request.set_prev_log_term(prev_log_term);
        request.set_commit_index(std::min(log_vars_.commit_index(), last_entry_index));

        auto s = AppendEntries(request, hp.ToString());
        assert(s.ok());
    }

    return Status::OK();
}

void
Raft::PrintId() const {
    LOG(INFO) << Node::GetInstance().id().ToString() << " : I am " << State2String(server_vars_.state());
    TraceLog("PrintId", __func__);
}

State
Raft::CurrentState() const {
    return server_vars_.state();
}

int64_t
Raft::CurrentTerm() const {
    return server_vars_.current_term().get();
}

bool
Raft::HasLeader() const {
    return (leader_ != 0);
}

void
Raft::BeFollower() {
    TraceLog("BeFollower", __func__);
    server_vars_.set_state(STATE_FOLLOWER);
    //leader_ = 0;

    ClearHeartbeatTimer();
    ResetElectionTimer();
}

void
Raft::Elect() {
    TraceLog("Elect", __func__);
    if (server_vars_.state() == STATE_FOLLOWER) {
        Follower2Candidate();
    }
    assert(server_vars_.state() == STATE_CANDIDATE);

    server_vars_.mutable_current_term().Next();
    server_vars_.mutable_vote_for().Clear();
    candidate_vars_.mutable_votes_granted().Reset(server_vars_.current_term().get());
    candidate_vars_.mutable_votes_responded().Reset(server_vars_.current_term().get());

    VoteForSelf();
    RequestVotePeers();
    ResetElectionTimer();
}

void
Raft::VoteForTerm(int64_t term, uint64_t node_id) {
    assert(term == server_vars_.current_term().get());
    assert(!server_vars_.vote_for().HasVoted());

    server_vars_.mutable_vote_for().Vote(node_id);
}

void
Raft::VoteForSelf() {
    VoteForTerm(server_vars_.current_term().get(), Node::GetInstance().id().code());

    vraft_rpc::RequestVoteReply reply;
    reply.set_term(server_vars_.current_term().get());
    reply.set_vote_granted(true);
    reply.set_node_id(Node::GetInstance().id().code());

    candidate_vars_.mutable_votes_granted().Vote(reply);
    candidate_vars_.mutable_votes_responded().Add(reply);
}

void
Raft::UpdateTerm(int64_t term) {
    TraceLog("UpdateTerm", __func__);

    if (term > server_vars_.current_term().get()) {
        server_vars_.mutable_current_term().set(term);
        BeFollower();
        server_vars_.mutable_vote_for().Clear();
    }
}

void
Raft::AdvanceCommitIndex() {
    TraceLog("AdvanceCommitIndex", __func__);
}

void
Raft::Follower2Candidate() {
    TraceLog("Follower2Candidate", __func__);
    assert(server_vars_.state() == STATE_FOLLOWER);
    server_vars_.set_state(STATE_CANDIDATE);
}

void
Raft::Candidate2Leader() {
    TraceLog("Candidate2Leader", __func__);
    assert(server_vars_.state() == STATE_CANDIDATE);
    assert(candidate_vars_.votes_granted().Majority());

    if (server_vars_.state() == STATE_CANDIDATE) {
        server_vars_.set_state(STATE_LEADER);
        leader_ = Node::GetInstance().id().code();

        for (auto &kv : leader_vars_.mutable_next_index()) {
            kv.second = Node::GetInstance().raft().log_vars().log().Len() + 1;
        }

        for (auto &kv : leader_vars_.mutable_match_index()) {
            kv.second = 0;
        }

        ClearElectionTimer();
        ResetHeartbeatTimer();
        auto s = AppendEntriesPeers();
        assert(s.ok());
    }
}

void
Raft::Leader2Follower() {
    TraceLog("Leader3Follower", __func__);

    assert(server_vars_.state() == STATE_LEADER);
    BeFollower();
}

void
Raft::Candidate2Follower() {
    TraceLog("Candidate2Follower", __func__);

    assert(server_vars_.state() == STATE_CANDIDATE);
    BeFollower();
}

void
Raft::ResetElectionTimer() {
    election_random_ms_ = util::random_int(Config::GetInstance().election_timeout(),
                                           2 * Config::GetInstance().election_timeout());
    if (-1 == election_timer_) {
        election_timer_= Env::GetInstance().timer()->RunAfter(
                             std::bind(&Raft::EqElect, this), election_random_ms_);
        if (election_timer_!= -1) {
            char buf[128];
            snprintf(buf, sizeof(buf), "election_timer_:%d, election_random_ms_:%d", election_timer_, election_random_ms_);
            LOG(INFO) << buf;
        }
    } else {
        auto s = Env::GetInstance().timer()->ResetRunAfter(election_timer_, election_random_ms_);
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
    int heartbeat_random_ms_ = Config::GetInstance().heartbeat_timeout();
    if (-1 == heartbeat_timer_) {
        heartbeat_timer_ = Env::GetInstance().timer()->RunEvery(
                               std::bind(&Raft::EqAppendEntriesPeers, this), heartbeat_random_ms_);
        assert(heartbeat_timer_ != -1);
    } else {
        auto s = Env::GetInstance().timer()->ResetRunEvery(heartbeat_timer_, heartbeat_random_ms_);
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
Raft::TimerToJson(int timerfd) const {
    jsonxx::json64 j;

    struct itimerspec curr_value;
    int r = timerfd_gettime(timerfd, &curr_value);
    if (r == 0) {
        ;
    } else {
        memset(&curr_value, 0, sizeof(curr_value));
    }

    j["timerfd"] = timerfd;
    j["it_interval"]["sec"] = curr_value.it_interval.tv_sec;
    j["it_interval"]["nsec"] = curr_value.it_interval.tv_nsec;
    j["it_value"]["sec"] = curr_value.it_value.tv_sec;
    j["it_value"]["nsec"] = curr_value.it_value.tv_nsec;

    return j;
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

    j["election_timer"] = TimerToJson(election_timer_);
    j["election_random_ms"] = election_random_ms_;
    j["heartbeat_timer"] = TimerToJson(heartbeat_timer_);
    j["heartbeat_random_ms"] = heartbeat_random_ms_;

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

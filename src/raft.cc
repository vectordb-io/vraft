#include "env.h"
#include "util.h"
#include "raft.h"

namespace vraft {

RequestVoteManager::RequestVoteManager(int quorum)
    :term_(-1),
     quorum_(quorum) {
}

RequestVoteManager::~RequestVoteManager() {
}

void
RequestVoteManager::Vote(const vraft_rpc::RequestVoteReply &reply) {
    //assert(reply.granted());
    assert(reply.term() == term_);
    //votes_.insert(std::pair<std::string, vraft_rpc::RequestVoteReply>(reply.address(), reply));
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
}


Raft::Raft()
    :state_(STATE_INVALID),
     current_term_(-1),
     vote_for_(""),
     leader_(""),
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
    return Status::OK();
}

void
Raft::OnRequestVote(const vraft_rpc::RequestVote &request, vraft_rpc::RequestVoteReply &reply) {
}

Status
Raft::RequestVote(const vraft_rpc::RequestVote &request, const std::string &address) {
    auto s = Env::GetInstance().AsyncRequestVote(
                 request,
                 address,
                 std::bind(&Raft::OnRequestVoteReply, this, std::placeholders::_1)
             );
    return s;
}

Status
Raft::OnRequestVoteReply(const vraft_rpc::RequestVoteReply &reply) {
}

Status
Raft::RequestVoteAll() {
    vraft_rpc::RequestVote request;
    request.set_term(current_term_);
    //request.set_address(Config::GetInstance().MyAddress()->ToString());
    request.set_last_log_index(Env::GetInstance().log()->LastLogId().index());
    request.set_last_log_term(Env::GetInstance().log()->LastLogId().term());

    /*
    for (auto &hp : Config::GetInstance().address_) {
        auto s = RequestVote(request, hp->ToString());
        assert(s.ok());
    }
    */
}

void
Raft::Elect() {
    assert(!HasLeader());
    assert(state_ == STATE_CANDIDATE);

    current_term_++;
    RequestVoteAll();
}

void
Raft::VoteForTerm(int64_t term, const std::string &node) {
    assert(term == current_term_);
    assert(!HasVoted());
    vote_for_ = node;
}

bool
Raft::HasVoted() const {
    return !(vote_for_ == "");
}

void
Raft::BeFollower() {
    state_ = STATE_FOLLOWER;
    ClearElectionTimer();
    ClearHeartbeatTimer();
    ResetF2CTimer();
}

void
Raft::ResetF2CTimer() {
    int timer_ms = util::random_int(Config::GetInstance().election_timeout(),
                                    2 * Config::GetInstance().election_timeout());
    if (-1 == follower2candidate_timer_) {
        follower2candidate_timer_ = Env::GetInstance().timer()->RunAfter(
                                        std::bind(&Raft::Follower2Candidate, this), timer_ms);
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
Raft::ResetElectionTimer() {
    int timer_ms = util::random_int(Config::GetInstance().election_timeout(),
                                    2 * Config::GetInstance().election_timeout());
    if (-1 == election_timer_) {
        election_timer_= Env::GetInstance().timer()->RunAfter(
                             std::bind(&Raft::Elect, this), timer_ms);
        assert(election_timer_!= -1);
    } else {
        auto s = Env::GetInstance().timer()->ResetRunAfter(election_timer_, timer_ms);
        assert(s.ok());
    }
}

void
Raft::ClearElectionTimer() {
    if (-1 != election_timer_) {
        Env::GetInstance().timer()->Stop(election_timer_);
    }
}

void
Raft::ResetHeartbeatTimer() {
}

void
Raft::ClearHeartbeatTimer() {
    if (-1 != heartbeat_timer_) {
        Env::GetInstance().timer()->Stop(heartbeat_timer_);
    }
}

void
Raft::Follower2Candidate() {
    assert(state_ == STATE_FOLLOWER);
    state_ = STATE_CANDIDATE;
    ClearF2CTimer();
    ClearHeartbeatTimer();
    ResetElectionTimer();
}

void
Raft::Candidate2Leader() {
    assert(state_ == STATE_CANDIDATE);
}

void
Raft::Leader2Follower() {
    assert(state_ == STATE_LEADER);
    BeFollower();
}

void
Raft::Candidate2Follower() {
    assert(state_ == STATE_CANDIDATE);
    BeFollower();
}

Status
Raft::CurrentTerm(int64_t &term) const {
    auto s = Env::GetInstance().CurrentTerm(term);
    return s;
}

Status
Raft::CurrentTermPersist(int64_t term) {
    auto s = Env::GetInstance().CurrentTermPersist(term);
    return s;
}

Status
Raft::VoteFor(std::string &vote_for) const {
    auto s = Env::GetInstance().VoteFor(vote_for);
    return s;
}

Status
Raft::VoteForPersist(const std::string &vote_for) {
    auto s = Env::GetInstance().VoteForPersist(vote_for);
    return s;
}

}  // namespace vraft

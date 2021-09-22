#ifndef __VRAFT_RAFT_H__
#define __VRAFT_RAFT_H__

#include <map>
#include <string>
#include "jsonxx/json.hpp"
#include "log.h"
#include "timer.h"
#include "nodeid.h"
#include "config.h"
#include "status.h"
#include "vraft_rpc.grpc.pb.h"

namespace vraft {

enum State {
    STATE_LEADER,
    STATE_CANDIDATE,
    STATE_FOLLOWER,
    STATE_INVALID,
};

std::string State2String(State s);

class VotesGranted {
  public:
    VotesGranted(int quorum);
    ~VotesGranted();
    VotesGranted(const VotesGranted&) = delete;
    VotesGranted& operator=(const VotesGranted&) = delete;

    void Vote(const vraft_rpc::RequestVoteReply &reply);
    bool Majority() const;
    void Reset(int64_t term);

    bool to_leader() const {
        return to_leader_;
    }

    void set_to_leader() {
        to_leader_ = true;
    }

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
    std::map<uint64_t, vraft_rpc::RequestVoteReply> votes_;
    int64_t term_;
    int quorum_;
    bool to_leader_;
};

class VotesResponded {
  public:
    VotesResponded();
    ~VotesResponded() = default;
    VotesResponded(const VotesResponded&) = delete;
    VotesResponded& operator=(const VotesResponded&) = delete;

    bool IsResponded(int64_t node_id) const;
    void Add(const vraft_rpc::RequestVoteReply &reply);
    void Reset(int64_t term);

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
    std::map<uint64_t, vraft_rpc::RequestVoteReply> responded_;
    std::set<uint64_t> unresponded_;
    std::set<uint64_t> all_nodes_;
    int64_t term_;
};

class PersistedTerm {
  public:
    PersistedTerm();
    ~PersistedTerm() = default;
    PersistedTerm(const PersistedTerm&) = delete;
    PersistedTerm& operator=(const PersistedTerm&) = delete;

    Status Init();
    void Next();
    int64_t get() const;
    void set(int64_t term);

  private:
    int64_t term_;
};

class PersistedVoteFor {
  public:
    PersistedVoteFor();
    ~PersistedVoteFor() = default;
    PersistedVoteFor(const PersistedVoteFor&) = delete;
    PersistedVoteFor& operator=(const PersistedVoteFor&) = delete;

    Status Init();
    bool HasVoted() const;
    void Vote(uint64_t node_id);
    void Clear();
    NodeId ToNodeId() const;
    uint64_t ToUInt() const;

  private:
    uint64_t node_id_;
};

class ServerVars {
  public:
    ServerVars() = default;
    ~ServerVars() = default;
    ServerVars(const ServerVars&) = delete;
    ServerVars& operator=(const ServerVars&) = delete;
    Status Init();

    State state() const {
        return state_;
    }

    void set_state(State state) {
        state_ = state;
    }

    const PersistedTerm& current_term() const {
        return current_term_;
    }

    PersistedTerm& mutable_current_term() {
        return current_term_;
    }

    const PersistedVoteFor& vote_for() const {
        return vote_for_;
    }

    PersistedVoteFor& mutable_vote_for() {
        return vote_for_;
    }

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
    State state_;
    PersistedTerm current_term_;
    PersistedVoteFor vote_for_;
};

class CandidateVars {
  public:
    CandidateVars(int quorum);
    ~CandidateVars() = default;
    CandidateVars(const CandidateVars&) = delete;
    CandidateVars& operator=(const CandidateVars&) = delete;
    Status Init();

    const VotesGranted& votes_granted() const {
        return votes_granted_;
    }

    VotesGranted& mutable_votes_granted() {
        return votes_granted_;
    }

    const VotesResponded& votes_responded() const {
        return votes_responded_;
    }

    VotesResponded& mutable_votes_responded() {
        return votes_responded_;
    }

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
    VotesGranted votes_granted_;
    VotesResponded votes_responded_;
};

class LeaderVars {
  public:
    LeaderVars() = default;
    ~LeaderVars() = default;
    LeaderVars(const LeaderVars&) = delete;
    LeaderVars& operator=(const LeaderVars&) = delete;
    Status Init();

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

    const std::map<uint64_t, int>& next_index() const {
        return next_index_;
    }

    std::map<uint64_t, int>& mutable_next_index() {
        return next_index_;
    }

    const std::map<uint64_t, int>& match_index() const {
        return match_index_;
    }

    std::map<uint64_t, int>& mutable_match_index() {
        return match_index_;
    }

  private:
    std::map<uint64_t, int> next_index_;
    std::map<uint64_t, int> match_index_;
};

class LogVars {
  public:
    LogVars(const std::string &log_path);
    ~LogVars() = default;
    LogVars(const LogVars&) = delete;
    LogVars& operator=(const LogVars&) = delete;
    Status Init();

    int commit_index() const {
        return commit_index_;
    }

    void set_commit_index(int commit_index) {
        commit_index_ = commit_index;
    }

    const Log& log() const {
        return log_;
    }

    Log& mutable_log() {
        return log_;
    }

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
    int commit_index_;
    Log log_;
};

class Raft {
  public:
    Raft();
    ~Raft();
    Raft(const Raft&) = delete;
    Raft& operator=(const Raft&) = delete;
    Status Init();
    Status Start();

    void OnClientRequest(const vraft_rpc::ClientRequest &request, void *async_flag);
    void OnRequestVote(const vraft_rpc::RequestVote &request, vraft_rpc::RequestVoteReply &reply);
    Status RequestVote(const vraft_rpc::RequestVote &request, const std::string &address);
    Status OnRequestVoteReply(const vraft_rpc::RequestVoteReply &reply);
    void OnAppendEntries(const vraft_rpc::AppendEntries &request, vraft_rpc::AppendEntriesReply &reply);
    Status AppendEntries(const vraft_rpc::AppendEntries &request, const std::string &address);
    Status OnAppendEntriesReply(const vraft_rpc::AppendEntriesReply &reply);

    Status RequestVotePeers();
    Status AppendEntriesPeers(void *async_flag);
    void PrintId() const;
    State CurrentState() const;
    int64_t CurrentTerm() const;
    bool HasLeader() const;

    const ServerVars& server_vars() const {
        return server_vars_;
    }

    const CandidateVars& candidate_vars() const {
        return candidate_vars_;
    }

    const LeaderVars& leader_vars() const {
        return leader_vars_;
    }

    const LogVars& log_vars() const {
        return log_vars_;
    }

  private:
    void BecomeFollower();
    void BecomeLeader();
    void Elect();

    void VoteForTerm(int64_t term, uint64_t node_id);
    void VoteForSelf();
    void UpdateTerm(int64_t term);
    void MaybeAdvanceCommitIndex();

    void Follower2Candidate();
    void Candidate2Leader();
    void Leader2Follower();
    void Candidate2Follower();

    void ResetElectionTimer();
    void ClearElectionTimer();
    void EqElectionTimeout();
    void ElectionTimeout();

    void ResetHeartbeatTimer();
    void ClearHeartbeatTimer();
    void EqHeartbeatTimeout();
    void HeartbeatTimeout();

    // for debug
    void TraceRequestVote(const vraft_rpc::RequestVote &msg, const std::string &address) const;
    void TraceOnRequestVote(const vraft_rpc::RequestVote &msg, const std::string &address) const;
    void TraceRequestVoteReply(const vraft_rpc::RequestVoteReply &msg, const std::string &address) const;
    void TraceOnRequestVoteReply(const vraft_rpc::RequestVoteReply &msg, const std::string &address) const;
    void TraceAppendEntries(const vraft_rpc::AppendEntries &msg, const std::string &address) const;
    void TraceOnAppendEntries(const vraft_rpc::AppendEntries &msg, const std::string &address) const;
    void TraceAppendEntriesReply(const vraft_rpc::AppendEntriesReply &msg, const std::string &address) const;
    void TraceOnAppendEntriesReply(const vraft_rpc::AppendEntriesReply &msg, const std::string &address) const;
    void TraceLog(const std::string &log_flag, const std::string func_name) const;

    jsonxx::json64 TimerToJson(int timerfd) const;
    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
    ServerVars server_vars_;
    CandidateVars candidate_vars_;
    LeaderVars leader_vars_;
    LogVars log_vars_;
    uint64_t leader_cache_;

    bool enable_election_timer_;
    int election_timer_;
    int election_random_ms_;

    bool enable_heartbeat_timer_;
    int heartbeat_timer_;
    int heartbeat_random_ms_;
};

}  // namespace vraft

#endif

#ifndef __VRAFT_RAFT_H__
#define __VRAFT_RAFT_H__

#include <map>
#include <string>
#include "jsonxx/json.hpp"
#include "timer.h"
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

class RequestVoteManager {
  public:
    RequestVoteManager(int quorum);
    ~RequestVoteManager();
    RequestVoteManager(const RequestVoteManager&) = delete;
    RequestVoteManager& operator=(const RequestVoteManager&) = delete;

    void Vote(const vraft_rpc::RequestVoteReply &reply);
    bool Majority() const;
    void Reset(int64_t term);
    std::string ToString() const;

  private:
    std::map<uint64_t, vraft_rpc::RequestVoteReply> votes_;
    int64_t term_;
    int quorum_;
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
    Status RequestVotePeers(int64_t term);
    Status RequestVotePeers();

    void OnAppendEntries(const vraft_rpc::AppendEntries &request, vraft_rpc::AppendEntriesReply &reply);
    Status AppendEntries(const vraft_rpc::AppendEntries &request, const std::string &address);
    Status OnAppendEntriesReply(const vraft_rpc::AppendEntriesReply &reply);
    Status AppendEntriesPeers();

    void PrintId() const;

    State state() const {
        return state_;
    }

    int64_t current_term() const {
        return current_term_;
    }

    bool HasLeader() const {
        return (leader_ != 0);
    }

  private:
    void BeFollower();
    void Elect();
    void VoteForTerm(int64_t term, uint64_t node_id);
    void VoteForSelf();
    bool HasVoted(int64_t term) const;
    void NextTerm();
    Status ClearVoteFor();
    void UpdateTerm(int64_t term);

    void ResetF2CTimer();
    void ClearF2CTimer();
    void EqFollower2Candidate();

    void ResetElectionTimer();
    void ClearElectionTimer();
    void EqElect();

    void ResetHeartbeatTimer();
    void ClearHeartbeatTimer();
    void EqAppendEntriesPeers();

    void Follower2Candidate();
    void Candidate2Leader();
    void Leader2Follower();
    void Candidate2Follower();

    Status CurrentTerm(int64_t &term) const;
    Status PersistCurrentTerm(int64_t term);
    Status VoteFor(uint64_t &node_id) const;
    Status PersistVoteFor(uint64_t node_id);

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

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
    State state_;
    int64_t current_term_;
    uint64_t vote_for_;
    uint64_t leader_;

    int follower2candidate_timer_;
    int election_timer_;
    int heartbeat_timer_;

    RequestVoteManager request_vote_manager_;
};

}  // namespace vraft

#endif

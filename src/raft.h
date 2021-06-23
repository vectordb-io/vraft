#ifndef __VRAFT_RAFT_H__
#define __VRAFT_RAFT_H__

#include <map>
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
    std::map<std::string, vraft_rpc::RequestVoteReply> votes_;
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

    void OnRequestVote(const vraft_rpc::RequestVote &request, vraft_rpc::RequestVoteReply &reply);
    Status RequestVote(const vraft_rpc::RequestVote &request, const std::string &address);
    Status OnRequestVoteReply(const vraft_rpc::RequestVoteReply &reply);
    Status RequestVoteAll();

    void BeFollower();

    State state() const {
        return state_;
    }

    int64_t current_term() const {
        return current_term_;
    }

    bool ImLeader() const {
        if (leader_ == Config::GetInstance().MyAddress()->ToString()) {
            assert(state_ == STATE_LEADER);
            return true;
        } else {
            return false;
        }
    }

    bool HasLeader() const {
        return (leader_ != "");
    }

    std::string master() const {
        assert(HasLeader());
        return leader_;
    }

  private:
    void ResetF2CTimer();
    void ClearF2CTimer();
    void ResetElectionTimer();
    void ClearElectionTimer();
    void ResetHeartbeatTimer();
    void ClearHeartbeatTimer();

    void Follower2Candidate();
    void Candidate2Leader();
    void Leader2Follower();
    void Candidate2Follower();

    void Elect();
    void VoteForTerm(int64_t term, const std::string &node);
    bool HasVoted() const;

    Status CurrentTerm(int64_t &term) const;
    Status CurrentTermPersist(int64_t term);
    Status VoteFor(std::string &vote_for) const;
    Status VoteForPersist(const std::string &vote_for);

    State state_;
    int64_t current_term_;
    std::string vote_for_;
    std::string leader_;

    int follower2candidate_timer_;
    int election_timer_;
    int heartbeat_timer_;

    RequestVoteManager request_vote_manager_;
};

}  // namespace vraft

#endif

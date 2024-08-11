#ifndef VRAFT_RAFT_H_
#define VRAFT_RAFT_H_

#include <functional>
#include <vector>

#include "append_entries.h"
#include "append_entries_reply.h"
#include "checker.h"
#include "client_request.h"
#include "config.h"
#include "config_manager.h"
#include "index_manager.h"
#include "install_snapshot.h"
#include "install_snapshot_reply.h"
#include "nlohmann/json.hpp"
#include "peer_manager.h"
#include "ping.h"
#include "ping_reply.h"
#include "raft_addr.h"
#include "raft_log.h"
#include "request_vote.h"
#include "request_vote_reply.h"
#include "simple_random.h"
#include "snapshot_manager.h"
#include "solid_data.h"
#include "state_machine.h"
#include "timeout_now.h"
#include "timer.h"
#include "timer_manager.h"
#include "tracer.h"
#include "vote_manager.h"

namespace vraft {

enum State {
  STATE_FOLLOWER = 0,
  STATE_CANDIDATE,
  STATE_LEADER,
  STATE_STANDBY,
  STATE_ERROR,
};

const char *StateToStr(enum State state);
void Tick(Timer *timer);
void Elect(Timer *timer);
void RequestVoteRpc(Timer *timer);
void HeartBeat(Timer *timer);

class Raft final {
 public:
  Raft(const std::string &path, const RaftConfig &rc);
  ~Raft();
  Raft(const Raft &r) = delete;
  Raft &operator=(const Raft &r) = delete;

  // life cycle
  int32_t Start();
  int32_t Stop();
  void Init();

  // client request
  int32_t Propose(std::string value, Functor cb);
  int32_t LeaderTransfer(RaftAddr &dest);
  int32_t LeaderTransferFirstPeer();

  // on message
  int32_t OnPing(struct Ping &msg);
  int32_t OnPingReply(struct PingReply &msg);
  int32_t OnRequestVote(struct RequestVote &msg);
  int32_t OnRequestVoteReply(struct RequestVoteReply &msg);
  int32_t OnAppendEntries(struct AppendEntries &msg);
  int32_t OnAppendEntriesReply(struct AppendEntriesReply &msg);
  int32_t OnInstallSnapshot(struct InstallSnapshot &msg);
  int32_t OnInstallSnapshotReply(struct InstallSnapshotReply &msg);
  int32_t OnTimeoutNow(struct TimeoutNow &msg);
  int32_t OnClientRequest(struct ClientRequest &msg,
                          vraft::TcpConnectionSPtr conn);

  // send message
  int32_t SendPing(uint64_t dest, Tracer *tracer);
  int32_t SendRequestVote(uint64_t dest, Tracer *tracer);
  int32_t SendAppendEntries(uint64_t dest, Tracer *tracer);
  int32_t SendInstallSnapshot(uint64_t dest, Tracer *tracer);
  int32_t SendRequestVoteReply(RequestVoteReply &msg, Tracer *tracer);
  int32_t SendAppendEntriesReply(AppendEntriesReply &msg, Tracer *tracer);
  int32_t SendInstallSnapshotReply(InstallSnapshotReply &msg, Tracer *tracer);
  int32_t SendTimeoutNow(uint64_t dest, bool force, Tracer *tracer);

  // utils
  int16_t Id() { return config_mgr_.Current().me.id(); }
  RaftAddr Me() { return config_mgr_.Current().me; }
  RaftTerm Term();
  std::vector<RaftAddr> Peers() { return config_mgr_.Current().peers; }
  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
  void Print(bool tiny, bool one_line);
  void DoElect(Tracer *tracer);
  void DoPreVote(Tracer *tracer);

  // get set
  bool started() const;
  enum State state();
  void set_send(SendFunc func);
  void set_make_timer(MakeTimerFunc func);
  void set_assert_loop(Functor assert_loop);
  void set_tracer_cb(TracerCb tracer_cb);
  void set_create_sm(CreateSMFunc func);
  void set_reader_sm(CreateReaderFunc func);
  void set_writer_sm(CreateWriterFunc func);
  bool print_screen() const;
  void set_print_screen(bool print_screen);
  bool enable_pre_vote() const;
  void set_enable_pre_vote(bool enable_pre_vote);
  StateMachineSPtr sm();

 private:
  bool IfSelfVote();
  int32_t InitConfig();

  void AppendNoop(Tracer *tracer);
  void MaybeCommit(Tracer *tracer);
  void BecomeLeader(Tracer *tracer);
  void StateMachineApply(Tracer *tracer);
  void StepDown(RaftTerm new_term, Tracer *tracer);

  RaftIndex LastIndex();
  RaftTerm LastTerm();
  RaftTerm GetTerm(RaftIndex index);

 private:
  bool started_;
  std::string home_path_;
  std::string conf_path_;
  std::string meta_path_;
  std::string log_path_;
  std::string sm_path_;

  int32_t seqid_;

  // raft state: every server
  enum State state_;
  RaftIndex commit_;
  RaftIndex last_apply_;

  RaftLog log_;
  SolidData meta_;

  RaftAddr leader_;  // leader cache
  ConfigManager config_mgr_;

  // raft state: candidate
  VoteManager vote_mgr_;

  // raft state: leader
  IndexManager index_mgr_;

  // raft state: state machine
  StateMachineSPtr sm_;

  // raft state: snapshot
  SnapshotManager snapshot_mgr_;

  // helper
  TimerManager timer_mgr_;
  PeerManager peer_mgr_;
  SendFunc send_;
  MakeTimerFunc make_timer_;
  Functor assert_loop_;
  TracerCb tracer_cb_;
  CreateSMFunc create_sm_;
  CreateReaderFunc create_reader_;
  CreateWriterFunc create_writer_;

  bool print_screen_;
  bool enable_pre_vote_;

  friend void Tick(Timer *timer);
  friend void Elect(Timer *timer);
  friend void RequestVoteRpc(Timer *timer);
  friend void HeartBeat(Timer *timer);
};

inline bool Raft::started() const { return started_; }

inline enum State Raft::state() { return state_; }

inline void Raft::set_send(SendFunc func) { send_ = func; }

inline void Raft::set_make_timer(MakeTimerFunc func) { make_timer_ = func; }

inline void Raft::set_assert_loop(Functor assert_loop) {
  assert_loop_ = assert_loop;
}

inline void Raft::set_tracer_cb(TracerCb tracer_cb) { tracer_cb_ = tracer_cb; }

inline void Raft::set_create_sm(CreateSMFunc func) { create_sm_ = func; }

inline void Raft::set_reader_sm(CreateReaderFunc func) {
  create_reader_ = func;
}

inline void Raft::set_writer_sm(CreateWriterFunc func) {
  create_writer_ = func;
}

inline bool Raft::print_screen() const { return print_screen_; }

inline void Raft::set_print_screen(bool print_screen) {
  print_screen_ = print_screen;
}

inline bool Raft::enable_pre_vote() const { return enable_pre_vote_; }

inline void Raft::set_enable_pre_vote(bool enable_pre_vote) {
  enable_pre_vote_ = enable_pre_vote;
}

inline StateMachineSPtr Raft::sm() { return sm_; }

}  // namespace vraft

#endif

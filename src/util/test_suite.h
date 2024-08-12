#ifndef VRAFT_TEST_SUITE_H_
#define VRAFT_TEST_SUITE_H_

#include <functional>
#include <unordered_map>

#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "state_machine.h"
#include "timer.h"

namespace vraft {

extern EventLoopSPtr gtest_loop;
extern RemuSPtr gtest_remu;
extern std::string gtest_path;

extern bool gtest_enable_pre_vote;

void RemuLogState(std::string key);
void PrintAndCheck();
void GenerateConfig(std::vector<Config> &configs, int32_t peers_num);
void GTestSignalHandler(int signal);

using GTestTickFunc = std::function<void(Timer *)>;

void RemuTestSetUp(std::string path, GTestTickFunc tick_func,
                   CreateSMFunc create_sm);
void RemuTestTearDown();
void RunRemuTest(int32_t node_num);

//------------------TestSM---------------------------

class TestSM : public vraft::StateMachine {
 public:
  TestSM(std::string path);
  ~TestSM();

  int32_t Restore() override;
  int32_t Apply(vraft::LogEntry *entry, vraft::RaftAddr addr) override;
  vraft::RaftIndex LastIndex() override;
  vraft::RaftTerm LastTerm() override;

  int32_t Get(const std::string &key, std::string &value);

 public:
  leveldb::DB *db;
};

//------------------TestSM---------------------------

StateMachineSPtr CreateSM(std::string &path);

//-----------------------------------------

class Timer;
using CondFunc = std::function<bool()>;

enum TestState {
  kTestState0 = 0,
  kTestState1,
  kTestState2,
  kTestState3,
  kTestState4,
  kTestState5,
  kTestState6,
  kTestState7,
  kTestState8,
  kTestState9,
  kTestStateEnd,
};

inline std::string TestState2Str(TestState state) {
  switch (state) {
    case kTestState0:
      return "kTestState0";
    case kTestState1:
      return "kTestState1";
    case kTestState2:
      return "kTestState2";
    case kTestState3:
      return "kTestState3";
    case kTestState4:
      return "kTestState4";
    case kTestState5:
      return "kTestState5";
    case kTestState6:
      return "kTestState6";
    case kTestState7:
      return "kTestState7";
    case kTestState8:
      return "kTestState8";
    case kTestState9:
      return "kTestState9";
    case kTestStateEnd:
      return "kTestStateEnd";
    default:
      return "UnknowState";
  }
}

struct StateChange {
  TestState next;
  CondFunc func;
};

extern TestState current_state;
extern std::unordered_map<TestState, StateChange> rules;

}  // namespace vraft

#endif

#ifndef VRAFT_TEST_SUITE_H_
#define VRAFT_TEST_SUITE_H_

#include <functional>
#include <unordered_map>

#include "timer.h"

namespace vraft {

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

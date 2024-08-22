#include <gtest/gtest.h>

#include <csignal>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#include "clock.h"
#include "coding.h"
#include "config.h"
#include "logger.h"
#include "raft_server.h"
#include "remu.h"
#include "test_suite.h"
#include "timer.h"
#include "util.h"
#include "vraft_logger.h"

vraft::RaftSPtr del_follower = nullptr;
vraft::RaftSPtr save_leader = nullptr;

void RemuTick(vraft::Timer *timer) {
  switch (vraft::current_state) {
    // wait until elect leader, then wait 5s to ensure leader stable
    case vraft::kTestState0: {
      vraft::PrintAndCheck();

      int32_t leader_num = 0;
      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_LEADER &&
            ptr->raft()->started()) {
          save_leader = ptr->raft();
          leader_num++;
        }
      }

      if (leader_num == 1) {
        timer->RepeatDecr();
        if (timer->repeat_counter() == 0) {
          vraft::current_state = vraft::kTestState1;
        }
      }

      break;
    }

    // remove server
    case vraft::kTestState1: {
      vraft::PrintAndCheck();

      // get a follower
      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_FOLLOWER &&
            ptr->raft()->started()) {
          del_follower = ptr->raft();
          break;
        }
      }

      if (!del_follower) {
        vraft::current_state = vraft::kTestStateEnd;
        break;
      }

      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_LEADER &&
            ptr->raft()->started()) {
          int32_t rv = ptr->raft()->RemoveServer(del_follower->Me());
          ASSERT_EQ(rv, 0);

          // update repeat counter
          timer->set_repeat_times(5);
          vraft::current_state = vraft::kTestState2;

          break;
        }
      }

      break;
    }

    // wait until elect leader, then wait 5s to ensure leader stable
    case vraft::kTestState2: {
      vraft::PrintAndCheck();

      int32_t leader_num = 0;
      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_LEADER &&
            ptr->raft()->started()) {
          leader_num++;
        }
      }

      if (leader_num == 1) {
        timer->RepeatDecr();
        if (timer->repeat_counter() == 0) {
          timer->set_repeat_times(5);
          vraft::current_state = vraft::kTestState3;
        }
      }

      break;
    }

    // check disrupt server
    case vraft::kTestState3: {
      vraft::PrintAndCheck();

      timer->RepeatDecr();
      if (timer->repeat_counter() == 0) {
        // stop server
        del_follower->Stop();
        timer->set_repeat_times(5);
        vraft::current_state = vraft::kTestState4;
      }
      break;
    }

    // wait 5s
    case vraft::kTestState4: {
      vraft::PrintAndCheck();

      timer->RepeatDecr();
      if (timer->repeat_counter() == 0) {
        vraft::current_state = vraft::kTestState5;
      }
      break;
    }

    // check log consistant
    case vraft::kTestState5: {
      uint32_t checksum = save_leader->log().LastCheck();
      printf("====log checksum:%X \n\n", checksum);
      for (auto &rs : vraft::gtest_remu->raft_servers) {
        auto sptr = rs->raft();
        if (sptr->Me().ToU64() != del_follower->Me().ToU64() &&
            sptr->Peers().size() > 0) {
          uint32_t checksum2 = sptr->log().LastCheck();
          ASSERT_EQ(checksum, checksum2);
        }
      }

      timer->set_repeat_times(10);
      vraft::current_state = vraft::kTestStateEnd;
      break;
    }

    // quit
    case vraft::kTestStateEnd: {
      vraft::PrintAndCheck();

      del_follower.reset();
      save_leader.reset();

      std::cout << "exit ..." << std::endl;
      vraft::gtest_remu->Stop();
      vraft::gtest_loop->Stop();
    }

    default:
      break;
  }
}

class RemuTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // std::string path = std::string("/tmp/") + __func__;
    vraft::RemuTestSetUp("/tmp/remu_test_dir", RemuTick, nullptr);
  }

  void TearDown() override { vraft::RemuTestTearDown(); }
};

TEST_F(RemuTest, RemuTest) { vraft::RunRemuTest(vraft::gtest_node_num); }

// only 1 node of 2 cannot elect
// this case can investigate term-increase while enable pre-vote or not

int main(int argc, char **argv) {
  vraft::RemuParseConfig(argc, argv);

  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);

  if (vraft::gtest_enable_pre_vote && vraft::gtest_interval_check) {
    return RUN_ALL_TESTS();
  }

  return 0;
}
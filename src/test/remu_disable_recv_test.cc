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

vraft::RaftSPtr leader_ptr;
vraft::RaftSPtr follower_ptr;
vraft::RaftTerm save_term;

void RemuTick(vraft::Timer *timer) {
  switch (vraft::current_state) {
    // wait until elect leader, then wait 5s to ensure leader stable
    case vraft::kTestState0: {
      vraft::PrintAndCheck();

      int32_t leader_num = 0;
      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_LEADER &&
            ptr->raft()->started()) {
          leader_num++;

          // save leader ptr
          leader_ptr = ptr->raft();

          // save term
          save_term = leader_ptr->Term();
        }
      }

      if (leader_num == 1) {
        timer->RepeatDecr();
        if (timer->repeat_counter() == 0) {
          timer->set_repeat_times(10);
          vraft::current_state = vraft::kTestState1;
        }
      }

      break;
    }

    // stop one follower's recv
    // wait 10s
    case vraft::kTestState1: {
      vraft::PrintAndCheck();

      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_FOLLOWER &&
            ptr->raft()->started()) {
          // save follower ptr
          follower_ptr = ptr->raft();

          // stop one follower's recv
          follower_ptr->DisableRecv();

          timer->set_repeat_times(10);
          vraft::current_state = vraft::kTestState2;
          break;
        }
      }

      break;
    }

    // wait 10s
    case vraft::kTestState2: {
      vraft::PrintAndCheck();

      timer->RepeatDecr();
      if (timer->repeat_counter() == 0) {
        timer->set_repeat_times(10);
        vraft::current_state = vraft::kTestState3;
      }

      break;
    }

    // enable recv
    case vraft::kTestState3: {
      vraft::PrintAndCheck();

      timer->RepeatDecr();
      if (timer->repeat_counter() == 0) {
        follower_ptr->EnableRecv();

        // update repeat counter
        timer->set_repeat_times(10);
        vraft::current_state = vraft::kTestState4;
      }

      break;
    }

    // wait 10s, check leader
    case vraft::kTestState4: {
      vraft::PrintAndCheck();

      if (vraft::gtest_enable_pre_vote && vraft::gtest_interval_check) {
        for (auto ptr : vraft::gtest_remu->raft_servers) {
          if (ptr->raft()->state() == vraft::STATE_LEADER &&
              ptr->raft()->started()) {
            // term not change
            ASSERT_EQ(ptr->raft()->Term(), save_term);

            // leader not change
            ASSERT_EQ(ptr->raft()->Me().ToString(),
                      leader_ptr->Me().ToString());

            // leader timers == 1
            ASSERT_EQ(vraft::gtest_remu->LeaderTimes(), 1);

            break;
          }
        }

      } else if (vraft::gtest_enable_pre_vote && !vraft::gtest_interval_check) {
        // only pre_vote is not enough
        for (auto ptr : vraft::gtest_remu->raft_servers) {
          if (ptr->raft()->state() == vraft::STATE_LEADER &&
              ptr->raft()->started()) {
            // term may change, or may not change
            EXPECT_GE(ptr->raft()->Term(), save_term);

            // leader timers >= 1
            EXPECT_GE(vraft::gtest_remu->LeaderTimes(), 1);

            break;
          }
        }

      } else if (!vraft::gtest_enable_pre_vote && vraft::gtest_interval_check) {
        for (auto ptr : vraft::gtest_remu->raft_servers) {
          if (ptr->raft()->state() == vraft::STATE_LEADER &&
              ptr->raft()->started()) {
            // new leader term > save_term
            EXPECT_GT(ptr->raft()->Term(), save_term);

            // leader may change, or may not change
            // leader timers > 1
            EXPECT_GT(vraft::gtest_remu->LeaderTimes(), 1);

            break;
          }
        }
      } else if (!vraft::gtest_enable_pre_vote &&
                 !vraft::gtest_interval_check) {
        for (auto ptr : vraft::gtest_remu->raft_servers) {
          if (ptr->raft()->state() == vraft::STATE_LEADER &&
              ptr->raft()->started()) {
            // new leader term > save_term
            EXPECT_GT(ptr->raft()->Term(), save_term);

            // leader may change, or may not change
            // leader timers > 1
            EXPECT_GT(vraft::gtest_remu->LeaderTimes(), 1);

            break;
          }
        }
      }

      timer->RepeatDecr();
      if (timer->repeat_counter() == 0) {
        vraft::current_state = vraft::kTestState5;
      }

      break;
    }

    // check log consistant
    case vraft::kTestState5: {
      uint32_t checksum =
          vraft::gtest_remu->raft_servers[0]->raft()->log().LastCheck();
      printf("====log checksum:%X \n\n", checksum);
      for (auto &rs : vraft::gtest_remu->raft_servers) {
        auto sptr = rs->raft();
        uint32_t checksum2 = sptr->log().LastCheck();
        ASSERT_EQ(checksum, checksum2);
      }

      vraft::current_state = vraft::kTestStateEnd;

      break;
    }

    // quit
    case vraft::kTestStateEnd: {
      vraft::PrintAndCheck();

      // import!! reset
      leader_ptr.reset();
      follower_ptr.reset();

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

void PrintDesc() {
  if (vraft::gtest_desc) {
    std::string desc;
    char buf[256];

    snprintf(buf, sizeof(buf), "step1: start %d nodes \n", vraft::gtest_node_num);
    desc.append(buf);

    snprintf(buf, sizeof(buf), "step2: wait for leader elect \n");
    desc.append(buf);

    snprintf(buf, sizeof(buf), "step3: check leader stable \n");
    desc.append(buf);

    snprintf(buf, sizeof(buf), "step4: check log consistant \n");
    desc.append(buf);

    snprintf(buf, sizeof(buf), "step5: quit \n");
    desc.append(buf);

    std::cout << desc;
    exit(0);
  }
}

int main(int argc, char **argv) {
  vraft::RemuParseConfig(argc, argv);
  PrintDesc();

  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
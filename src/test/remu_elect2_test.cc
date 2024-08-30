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

    // stop leader
    case vraft::kTestState1: {
      vraft::PrintAndCheck();

      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_LEADER &&
            ptr->raft()->started()) {
          ptr->raft()->Stop();

          // update repeat counter
          timer->set_repeat_times(5);
          vraft::current_state = vraft::kTestState2;
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
          vraft::current_state = vraft::kTestState3;
        }
      }

      break;
    }

    // start the old leader
    case vraft::kTestState3: {
      vraft::PrintAndCheck();

      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (!ptr->raft()->started()) {
          int32_t rv = ptr->raft()->Start();
          ASSERT_EQ(rv, 0);
        }
      }

      // update repeat counter
      timer->set_repeat_times(5);
      vraft::current_state = vraft::kTestState4;

      break;
    }

    // leader not change, old leader become follower
    case vraft::kTestState4: {
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
          vraft::current_state = vraft::kTestState5;
        }
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

    snprintf(buf, sizeof(buf), "step1: start nodes \n", vraft::gtest_node_num);
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
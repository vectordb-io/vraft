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
    // wait until elect leader
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
        timer->set_repeat_times(5);
        vraft::current_state = vraft::kTestState1;
      }

      break;
    }

    // wait 5s to ensure leader stable
    case vraft::kTestState1: {
      vraft::PrintAndCheck();

      static bool goto_end = false;
      timer->RepeatDecr();
      if (timer->repeat_counter() == 0) {
        if (!goto_end) {
          goto_end = true;
          vraft::current_state = vraft::kTestState2;
        } else {
          goto_end = false;
          vraft::current_state = vraft::kTestStateEnd;
        }
      }

      break;
    }

    // leader transfer
    case vraft::kTestState2: {
      vraft::PrintAndCheck();

      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_LEADER &&
            ptr->raft()->started()) {
          ptr->raft()->LeaderTransferFirstPeer();
          vraft::current_state = vraft::kTestState0;
        }
      }

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

TEST_F(RemuTest, RunNode5) { vraft::RunRemuTest(5); }

TEST_F(RemuTest, RunNode4) { vraft::RunRemuTest(4); }

TEST_F(RemuTest, RunNode3) { vraft::RunRemuTest(3); }

TEST_F(RemuTest, RunNode2) { vraft::RunRemuTest(2); }

TEST_F(RemuTest, RunNode1) { vraft::RunRemuTest(1); }

int main(int argc, char **argv) {
  vraft::RemuParseConfig(argc, argv);

  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
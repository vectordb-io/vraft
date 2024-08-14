#include <gtest/gtest.h>

#include <csignal>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#include "clock.h"
#include "coding.h"
#include "common.h"
#include "config.h"
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "logger.h"
#include "raft_server.h"
#include "remu.h"
#include "state_machine.h"
#include "test_suite.h"
#include "timer.h"
#include "util.h"
#include "vraft_logger.h"

int32_t g_value_index = 0;
int32_t g_server_index = 0;
int32_t node_num = 3;
int32_t first_repeat_times = 1;

void RemuTick(vraft::Timer *timer) {
  switch (vraft::current_state) {
    // stop first one
    case vraft::kTestState0: {
      vraft::gtest_remu->raft_servers[(g_server_index++) % node_num]
          ->raft()
          ->Stop();
      vraft::current_state = vraft::kTestState1;
      timer->set_repeat_times(first_repeat_times);
      break;
    }

    // start one
    // stop next one
    case vraft::kTestState1: {
      vraft::PrintAndCheck();

      // start one
      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (!ptr->raft()->started()) {
          int32_t rv = ptr->raft()->Start();
          ASSERT_EQ(rv, 0);
        }
      }

      // stop next one
      vraft::gtest_remu->raft_servers[(g_server_index++) % node_num]
          ->raft()
          ->Stop();

      timer->RepeatDecr();

      if (timer->repeat_counter() == 0) {
        vraft::current_state = vraft::kTestState2;
      }

      break;
    }

    // start one
    case vraft::kTestState2: {
      vraft::PrintAndCheck();

      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (!ptr->raft()->started()) {
          int32_t rv = ptr->raft()->Start();
          ASSERT_EQ(rv, 0);
        }
      }

      // update repeat counter
      timer->set_repeat_times(5);
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

TEST_F(RemuTest, RunNode5) { vraft::RunRemuTest(node_num); }

int main(int argc, char **argv) {
  if (argc >= 2 && std::string(argv[1]) == std::string("--enable-pre-vote")) {
    vraft::gtest_enable_pre_vote = true;
  } else {
    vraft::gtest_enable_pre_vote = false;
  }

  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
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

    // propose 5 values
    case vraft::kTestState1: {
      vraft::PrintAndCheck();

      for (auto &rs : vraft::gtest_remu->raft_servers) {
        auto sptr = rs->raft();
        if (sptr && sptr->state() == vraft::STATE_LEADER && sptr->started()) {
          char value_buf[128];
          snprintf(value_buf, sizeof(value_buf), "key_%ld:value_%ld",
                   timer->repeat_counter(), timer->repeat_counter());
          int32_t rv = sptr->Propose(std::string(value_buf), nullptr);
          if (rv == 0) {
            printf("%s propose value: %s\n\n", sptr->Me().ToString().c_str(),
                   value_buf);
          }
          timer->RepeatDecr();
        }
      }

      if (timer->repeat_counter() == 0) {
        timer->set_repeat_times(2);
        vraft::current_state = vraft::kTestState2;
      }
      break;
    }

    // wait 2s, for log catch up
    case vraft::kTestState2: {
      vraft::PrintAndCheck();

      timer->RepeatDecr();
      if (timer->repeat_counter() == 0) {
        vraft::current_state = vraft::kTestState3;
      }

      break;
    }

    // stop a follower
    case vraft::kTestState3: {
      vraft::PrintAndCheck();

      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (ptr->raft()->state() == vraft::STATE_FOLLOWER &&
            ptr->raft()->started()) {
          ptr->raft()->Stop();

          // update repeat counter
          timer->set_repeat_times(5);
          vraft::current_state = vraft::kTestState4;

          break;
        }
      }

      break;
    }

    // propose 5 values
    case vraft::kTestState4: {
      vraft::PrintAndCheck();

      for (auto &rs : vraft::gtest_remu->raft_servers) {
        auto sptr = rs->raft();
        if (sptr && sptr->state() == vraft::STATE_LEADER && sptr->started()) {
          char value_buf[128];
          snprintf(value_buf, sizeof(value_buf), "key_%ld:value_%ld",
                   timer->repeat_counter() + 5, timer->repeat_counter() + 5);
          int32_t rv = sptr->Propose(std::string(value_buf), nullptr);
          if (rv == 0) {
            printf("%s propose value: %s\n\n", sptr->Me().ToString().c_str(),
                   value_buf);
          }
          timer->RepeatDecr();
        }
      }

      if (timer->repeat_counter() == 0) {
        timer->set_repeat_times(2);
        vraft::current_state = vraft::kTestState5;
      }
      break;
    }

    // start the follower
    case vraft::kTestState5: {
      vraft::PrintAndCheck();

      for (auto ptr : vraft::gtest_remu->raft_servers) {
        if (!ptr->raft()->started()) {
          int32_t rv = ptr->raft()->Start();
          ASSERT_EQ(rv, 0);
        }
      }

      // update repeat counter
      timer->set_repeat_times(5);
      vraft::current_state = vraft::kTestState6;

      break;
    }

    // wait 5s, for log catch up
    case vraft::kTestState6: {
      vraft::PrintAndCheck();

      timer->RepeatDecr();
      if (timer->repeat_counter() == 0) {
        vraft::current_state = vraft::kTestState7;
      }

      break;
    }

    // check sm value
    case vraft::kTestState7: {
      for (auto ptr : vraft::gtest_remu->raft_servers) {
        std::cout << "------------------------------------" << std::endl;
        std::cout << "LastIndex: " << ptr->raft()->sm()->LastIndex()
                  << std::endl;
        std::cout << "LastTerm: " << ptr->raft()->sm()->LastTerm() << std::endl;
        for (int i = 1; i <= 10; ++i) {
          char key[32];
          snprintf(key, sizeof(key), "key_%d", i);
          std::string value;
          vraft::TestSM *psm = (vraft::TestSM *)(ptr->raft()->sm().get());
          int32_t rv = psm->Get(std::string(key), value);
          ASSERT_EQ(rv, 1);
          std::cout << key << " --- " << value << std::endl;

          char value_buf[128];
          snprintf(value_buf, sizeof(value_buf), "value_%d", i);
          ASSERT_EQ(value, std::string(value_buf));
        }
      }

      vraft::current_state = vraft::kTestState8;
      break;
    }

    // check log consistant
    case vraft::kTestState8: {
      uint32_t checksum =
          vraft::gtest_remu->raft_servers[0]->raft()->log().LastCheck();
      printf("====log checksum:%X \n\n", checksum);
      for (auto &rs : vraft::gtest_remu->raft_servers) {
        auto sptr = rs->raft();
        uint32_t checksum2 = sptr->log().LastCheck();
        ASSERT_EQ(checksum, checksum2);
      }

      vraft::current_state = vraft::kTestState9;

      break;
    }

    // check sm consistant
    case vraft::kTestState9: {
      vraft::TestSM *psm =
          (vraft::TestSM
               *)(vraft::gtest_remu->raft_servers[0]->raft()->sm().get());
      int32_t apply_count = psm->apply_count();
      int32_t check_sum = psm->check_sum();
      std::string all_values = psm->all_values();
      std::cout << "first one:" << std::endl;
      std::cout << psm->ToJsonString(false, false) << std::endl;

      for (auto &rs : vraft::gtest_remu->raft_servers) {
        auto sptr = rs->raft();
        vraft::TestSM *psm2 = (vraft::TestSM *)(rs->raft()->sm().get());
        std::cout << "compare:" << (void *)(psm2) << std::endl;
        std::cout << psm2->ToJsonString(false, false) << std::endl;

        int32_t apply_count2 = psm2->apply_count();
        int32_t check_sum2 = psm2->check_sum();
        std::string all_values2 = psm2->all_values();
        ASSERT_EQ(apply_count, apply_count2);
        ASSERT_EQ(check_sum, check_sum2);
        ASSERT_EQ(all_values, all_values2);
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
    vraft::RemuTestSetUp("/tmp/remu_test_dir", RemuTick, vraft::CreateSM);
  }

  void TearDown() override { vraft::RemuTestTearDown(); }
};

TEST_F(RemuTest, RunNode3) { vraft::RunRemuTest(3); }

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
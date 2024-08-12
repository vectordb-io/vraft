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
          vraft::current_state = vraft::kTestStateEnd;
        }
      }

      break;
    }

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
    std::cout << "setting up test... \n";
    std::fflush(nullptr);
    vraft::gtest_path = "/tmp/remu_test_dir";
    std::string cmd = "rm -rf " + vraft::gtest_path;
    system(cmd.c_str());

    vraft::LoggerOptions logger_options{
        "vraft", false, 1, 8192, vraft::kLoggerTrace, true};
    std::string log_file = vraft::gtest_path + "/log/remu.log";
    vraft::vraft_logger.Init(log_file, logger_options);

    std::signal(SIGINT, vraft::GTestSignalHandler);
    vraft::CodingInit();

    assert(!vraft::gtest_loop);
    assert(!vraft::gtest_remu);
    vraft::gtest_loop = std::make_shared<vraft::EventLoop>("remu-loop");
    int32_t rv = vraft::gtest_loop->Init();
    ASSERT_EQ(rv, 0);

    vraft::gtest_remu = std::make_shared<vraft::Remu>(
        vraft::gtest_loop, vraft::gtest_enable_pre_vote);
    vraft::gtest_remu->tracer_cb = vraft::RemuLogState;

    vraft::TimerParam param;
    param.timeout_ms = 0;
    param.repeat_ms = 1000;
    param.cb = RemuTick;
    param.data = nullptr;
    param.name = "remu-timer";
    param.repeat_times = 10;
    vraft::gtest_loop->AddTimer(param);

    // important !!
    vraft::current_state = vraft::kTestState0;
  }

  void TearDown() override {
    std::cout << "tearing down test... \n";
    std::fflush(nullptr);

    vraft::gtest_remu->Clear();
    vraft::gtest_remu.reset();
    vraft::gtest_loop.reset();
    vraft::Logger::ShutDown();

    // system("rm -rf /tmp/remu_test_dir");
  }
};

void ElectTest(int32_t node_num) {
  int32_t peers_num = node_num - 1;
  vraft::GenerateConfig(vraft::gtest_remu->configs, peers_num);
  vraft::gtest_remu->Create();
  vraft::gtest_remu->Start();

  {
    vraft::EventLoopSPtr l = vraft::gtest_loop;
    std::thread t([l]() { l->Loop(); });
    l->WaitStarted();
    t.join();
  }

  std::cout << "join thread... \n";
  std::fflush(nullptr);
}

TEST_F(RemuTest, Elect5) { ElectTest(5); }
TEST_F(RemuTest, Elect4) { ElectTest(4); }
TEST_F(RemuTest, Elect3) { ElectTest(3); }
TEST_F(RemuTest, Elect2) { ElectTest(2); }
TEST_F(RemuTest, Elect1) { ElectTest(1); }

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
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

vraft::EventLoopSPtr loop;
vraft::RemuSPtr remu;
std::string test_path;

void RemuLogState(std::string key) {
  if (remu) {
    remu->Log(key);
  }
}

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << std::endl;
  std::cout << "exit ..." << std::endl;
  loop->RunFunctor(std::bind(&vraft::Remu::Stop, remu.get()));
  loop->Stop();
}

void RemuTick(vraft::Timer *timer) {
  switch (vraft::current_state) {
    case vraft::kTestState0: {
      remu->Print();
      remu->Check();
      int32_t leader_num = 0;
      for (auto ptr : remu->raft_servers) {
        if (ptr->raft()->state() == vraft::LEADER && ptr->raft()->started()) {
          leader_num++;
        }
      }

      if (leader_num == 1) {
        static int32_t leader_tick = 5;
        if (leader_tick-- == 0) {
          vraft::current_state = vraft::kTestState1;
        }
      }

      break;
    }

    case vraft::kTestState1: {
      remu->Print();
      remu->Check();
      for (auto ptr : remu->raft_servers) {
        if (ptr->raft()->state() == vraft::LEADER && ptr->raft()->started()) {
          ptr->raft()->Stop();
          vraft::current_state = vraft::kTestState2;
        }
      }

      break;
    }

    case vraft::kTestState2: {
      remu->Print();
      remu->Check();
      int32_t leader_num = 0;
      for (auto ptr : remu->raft_servers) {
        if (ptr->raft()->state() == vraft::LEADER && ptr->raft()->started()) {
          leader_num++;
        }
      }

      if (leader_num == 1) {
        static int32_t leader_tick2 = 5;
        if (leader_tick2-- == 0) {
          vraft::current_state = vraft::kTestState3;
        }
      }

      break;
    }

    case vraft::kTestState3: {
      remu->Print();
      remu->Check();
      for (auto ptr : remu->raft_servers) {
        if (!ptr->raft()->started()) {
          int32_t rv = ptr->raft()->Start();
          ASSERT_EQ(rv, 0);
        }
      }
      vraft::current_state = vraft::kTestState4;

      break;
    }

    case vraft::kTestState4: {
      remu->Print();
      remu->Check();
      int32_t leader_num = 0;
      for (auto ptr : remu->raft_servers) {
        if (ptr->raft()->state() == vraft::LEADER && ptr->raft()->started()) {
          leader_num++;
        }
      }

      if (leader_num == 1) {
        static int32_t leader_tick4 = 5;
        if (leader_tick4-- == 0) {
          vraft::current_state = vraft::kTestStateEnd;
        }
      }

      break;
    }

    case vraft::kTestStateEnd: {
      remu->Print();
      remu->Check();

      std::cout << "exit ..." << std::endl;
      remu->Stop();
      loop->Stop();
    }

    default:
      break;
  }
}

void GenerateConfig(std::vector<vraft::Config> &configs, int32_t peers_num) {
  configs.clear();
  vraft::GetConfig().peers().clear();
  vraft::GetConfig().set_my_addr(vraft::HostPort("127.0.0.1", 9000));
  for (int i = 1; i <= peers_num; ++i) {
    vraft::GetConfig().peers().push_back(
        vraft::HostPort("127.0.0.1", 9000 + i));
  }
  vraft::GetConfig().set_log_level(vraft::kLoggerTrace);
  vraft::GetConfig().set_enable_debug(true);
  vraft::GetConfig().set_path(test_path);
  vraft::GetConfig().set_mode(vraft::kSingleMode);

  vraft::GenerateRotateConfig(configs);
  std::cout << "generate configs, size:" << configs.size() << std::endl;
}

class RemuTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::cout << "setting up test... \n";
    std::fflush(nullptr);
    // test_path = "/tmp/remu_test_dir_" +
    // vraft::NsToString2(vraft::Clock::NSec());
    test_path = "/tmp/remu_test_dir";
    std::string cmd = "rm -rf " + test_path;
    system(cmd.c_str());

    vraft::LoggerOptions logger_options{
        "vraft", false, 1, 8192, vraft::kLoggerTrace, true};
    std::string log_file = test_path + "/log/remu.log";
    vraft::vraft_logger.Init(log_file, logger_options);

    std::signal(SIGINT, SignalHandler);
    vraft::CodingInit();

    assert(!loop);
    assert(!remu);
    loop = std::make_shared<vraft::EventLoop>("remu-loop");
    int32_t rv = loop->Init();
    ASSERT_EQ(rv, 0);

    remu = std::make_shared<vraft::Remu>(loop);
    remu->tracer_cb = RemuLogState;

    vraft::TimerParam param;
    param.timeout_ms = 0;
    param.repeat_ms = 1000;
    param.cb = RemuTick;
    param.data = nullptr;
    param.name = "remu-timer";
    param.repeat_times = 10;
    loop->AddTimer(param);

    // important !!
    vraft::current_state = vraft::kTestState0;
  }

  void TearDown() override {
    std::cout << "tearing down test... \n";
    std::fflush(nullptr);

    remu->Clear();
    remu.reset();
    loop.reset();
    vraft::Logger::ShutDown();

    // system("rm -rf /tmp/remu_test_dir");
  }
};

#if 0
TEST_F(RemuTest, Elect5) {
  GenerateConfig(remu->configs, 4);
  remu->Create();
  remu->Start();

  {
    vraft::EventLoopSPtr l = loop;
    std::thread t([l]() { l->Loop(); });
    l->WaitStarted();
    t.join();
  }

  std::cout << "join thread... \n";
  std::fflush(nullptr);
}
#endif

TEST_F(RemuTest, Elect3) {
  GenerateConfig(remu->configs, 2);
  remu->Create();
  remu->Start();

  {
    vraft::EventLoopSPtr l = loop;
    std::thread t([l]() { l->Loop(); });
    l->WaitStarted();
    t.join();
  }

  std::cout << "join thread... \n";
  std::fflush(nullptr);
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
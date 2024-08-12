#include "test_suite.h"

#include <gtest/gtest.h>

#include <csignal>

#include "common.h"
#include "eventloop.h"
#include "raft.h"
#include "remu.h"
#include "timer.h"

namespace vraft {

EventLoopSPtr gtest_loop;
RemuSPtr gtest_remu;
std::string gtest_path;

bool gtest_enable_pre_vote;

void RemuLogState(std::string key) {
  if (gtest_remu) {
    gtest_remu->Log(key);
  }
}

void PrintAndCheck() {
  printf("--- %s ---\n", TestState2Str(current_state).c_str());
  gtest_remu->Print();
  gtest_remu->Check();
}

void GenerateConfig(std::vector<Config> &configs, int32_t peers_num) {
  configs.clear();
  GetConfig().peers().clear();
  GetConfig().set_my_addr(HostPort("127.0.0.1", 9000));
  for (int i = 1; i <= peers_num; ++i) {
    GetConfig().peers().push_back(HostPort("127.0.0.1", 9000 + i));
  }
  GetConfig().set_log_level(kLoggerTrace);
  GetConfig().set_enable_debug(true);
  GetConfig().set_path(gtest_path);
  GetConfig().set_mode(kSingleMode);

  GenerateRotateConfig(configs);
  std::cout << "generate configs, size:" << configs.size() << std::endl;
}

void GTestSignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << std::endl;
  std::cout << "exit ..." << std::endl;
  gtest_loop->RunFunctor(std::bind(&vraft::Remu::Stop, gtest_remu.get()));
  gtest_loop->Stop();
}

void RemuTestSetUp(std::string path, GTestTickFunc tick_func) {
  std::cout << "setting up test... \n";
  std::fflush(nullptr);
  vraft::gtest_path = path;
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
  param.cb = tick_func;
  param.data = nullptr;
  param.name = "remu-timer";
  param.repeat_times = 10;
  vraft::gtest_loop->AddTimer(param);

  // important !!
  vraft::current_state = vraft::kTestState0;
}

void RemuTestTearDown() {
  std::cout << "tearing down test... \n";
  std::fflush(nullptr);

  vraft::gtest_remu->Clear();
  vraft::gtest_remu.reset();
  vraft::gtest_loop.reset();
  vraft::Logger::ShutDown();

  // system("rm -rf /tmp/remu_test_dir");
}

void RunRemuTest(int32_t node_num) {
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

//-----------------------------------------

TimerFunctor timer_func;
TestState current_state = kTestState0;
std::unordered_map<TestState, StateChange> rules;

bool HasLeader() { return true; }

void InitRemuTest() {
  rules[kTestState0].next = kTestStateEnd;
  rules[kTestState0].func = HasLeader;
}

}  // namespace vraft

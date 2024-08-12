#include "test_suite.h"

#include <gtest/gtest.h>

#include <csignal>

#include "common.h"
#include "eventloop.h"
#include "raft.h"
#include "remu.h"
#include "timer.h"
#include "util.h"

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
  gtest_loop->RunFunctor(std::bind(&Remu::Stop, gtest_remu.get()));
  gtest_loop->Stop();
}

StateMachineSPtr CreateSM(std::string &path) {
  StateMachineSPtr sptr(new TestSM(path));
  return sptr;
}

void RemuTestSetUp(std::string path, GTestTickFunc tick_func,
                   CreateSMFunc create_sm) {
  std::cout << "setting up test... \n";
  std::fflush(nullptr);
  gtest_path = path;
  std::string cmd = "rm -rf " + gtest_path;
  system(cmd.c_str());

  LoggerOptions logger_options{"vraft", false, 1, 8192, kLoggerTrace, true};
  std::string log_file = gtest_path + "/log/remu.log";
  vraft_logger.Init(log_file, logger_options);

  std::signal(SIGINT, GTestSignalHandler);
  CodingInit();

  assert(!gtest_loop);
  assert(!gtest_remu);
  gtest_loop = std::make_shared<EventLoop>("remu-loop");
  int32_t rv = gtest_loop->Init();
  ASSERT_EQ(rv, 0);

  gtest_remu = std::make_shared<Remu>(gtest_loop, gtest_enable_pre_vote);
  gtest_remu->tracer_cb = RemuLogState;
  gtest_remu->create_sm = create_sm;

  TimerParam param;
  param.timeout_ms = 0;
  param.repeat_ms = 1000;
  param.cb = tick_func;
  param.data = nullptr;
  param.name = "remu-timer";
  param.repeat_times = 5;
  gtest_loop->AddTimer(param);

  // important !!
  current_state = kTestState0;
}

void RemuTestTearDown() {
  std::cout << "tearing down test... \n";
  std::fflush(nullptr);

  gtest_remu->Clear();
  gtest_remu.reset();
  gtest_loop.reset();
  Logger::ShutDown();

  // system("rm -rf /tmp/remu_test_dir");
}

void RunRemuTest(int32_t node_num) {
  int32_t peers_num = node_num - 1;
  GenerateConfig(gtest_remu->configs, peers_num);
  gtest_remu->Create();
  gtest_remu->Start();

  {
    EventLoopSPtr l = gtest_loop;
    std::thread t([l]() { l->Loop(); });
    l->WaitStarted();
    t.join();
  }

  std::cout << "join thread... \n";
  std::fflush(nullptr);
}

//------------------TestSM---------------------------

const std::string last_index_key = "LAST_INDEX_KEY";
const std::string last_term_key = "LAST_TERM_KEY";

TestSM::TestSM(std::string path) : StateMachine(path) {
  leveldb::Options o;
  o.create_if_missing = true;
  o.error_if_exists = false;
  leveldb::Status status = leveldb::DB::Open(o, path, &db);
  assert(status.ok());
}

TestSM::~TestSM() { delete db; }

int32_t TestSM::Restore() {
  printf("\n\n****** TestSM Restore ****** path:%s \n\n", path().c_str());
  fflush(nullptr);
  return 0;
}

int32_t TestSM::Get(const std::string &key, std::string &value) {
  leveldb::ReadOptions ro;
  leveldb::Status s;
  s = db->Get(ro, leveldb::Slice(key), &value);
  if (s.ok()) {
    return 0;
  } else {
    return -1;
  }
}

// format: key:value
int32_t TestSM::Apply(LogEntry *entry, RaftAddr addr) {
  printf("\n\n****** TestSM Apply %s ****** entry:%s \n\n",
         addr.ToString().c_str(), entry->ToJsonString(true, true).c_str());
  fflush(nullptr);

  leveldb::WriteBatch batch;

  {
    char buf[sizeof(uint32_t)];
    EncodeFixed32(buf, entry->index);
    batch.Put(leveldb::Slice(last_index_key),
              leveldb::Slice(buf, sizeof(uint32_t)));
  }

  {
    char buf[sizeof(uint64_t)];
    EncodeFixed64(buf, entry->append_entry.term);
    batch.Put(leveldb::Slice(last_term_key),
              leveldb::Slice(buf, sizeof(uint64_t)));
  }

  std::vector<std::string> kv;
  Split(entry->append_entry.value, ':', kv);
  assert(kv.size() == 2);
  batch.Put(leveldb::Slice(kv[0]), leveldb::Slice(kv[1]));

  leveldb::WriteOptions wo;
  wo.sync = true;
  leveldb::Status s = db->Write(wo, &batch);
  assert(s.ok());

  return 0;
}

RaftIndex TestSM::LastIndex() {
  leveldb::ReadOptions ro;
  leveldb::Status s;
  std::string value;
  s = db->Get(ro, leveldb::Slice(last_index_key), &value);
  if (s.ok()) {
    assert(value.size() == sizeof(uint32_t));
    uint32_t u32 = DecodeFixed32(value.c_str());
    return u32;
  } else {
    return 0;
  }
}

RaftTerm TestSM::LastTerm() {
  leveldb::ReadOptions ro;
  leveldb::Status s;
  std::string value;
  s = db->Get(ro, leveldb::Slice(last_term_key), &value);
  if (s.ok()) {
    assert(value.size() == sizeof(uint64_t));
    uint64_t u64 = DecodeFixed64(value.c_str());
    return u64;
  } else {
    return 0;
  }
}

//------------------TestSM---------------------------

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

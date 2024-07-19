#include "remu.h"

#include <cstdio>
#include <unordered_map>

#include "clock.h"
#include "raft_server.h"
#include "util.h"

namespace vraft {

void Remu::Log(std::string key) {
  uint64_t ts = Clock::NSec();
  char ts_buf[64];
  if (key.empty()) {
    ts = Clock::NSec();
    snprintf(ts_buf, sizeof(ts_buf), "0x%lX", ts);
  } else {
    snprintf(ts_buf, sizeof(ts_buf), "%s", key.c_str());
  }

  std::string str;
  str.append("\n");
  str.append(ts_buf);
  str.append(" global-state: ");
  str.append(NsToString(ts));

  for (auto ptr : raft_servers) {
    str.append("\n");
    str.append(ts_buf);
    str.append(" global-state: ");
    str.append(ptr->raft()->ToJsonString(true, true));
  }
  vraft_logger.FInfo("%s", str.c_str());
}

void Remu::Print(bool tiny, bool one_line) {
  printf("--- remu global-state --- %s ---:\n",
         NsToString(Clock::NSec()).c_str());
  for (auto ptr : raft_servers) {
    ptr->Print(tiny, one_line);
    if (!one_line) {
      printf("\n");
    }
  }
  printf("\n");
  fflush(nullptr);
}

void Remu::Check() {
  CheckLeader();
  CheckLog();
  CheckMeta();
  CheckIndex();
}

void Remu::CheckLeader() {
  std::unordered_map<RaftTerm, RaftAddr> leader_map;
  for (auto &raft_server : raft_servers) {
    if (raft_server->raft()->state() == LEADER) {
      RaftTerm term = raft_server->raft()->Term();
      RaftAddr addr = raft_server->raft()->Me();
      auto it = leader_map.find(term);
      if (it == leader_map.end()) {
        leader_map[term] = addr;
      } else {
        std::cout << "check error, term:" << term
                  << ", leader:" << it->second.ToString() << " "
                  << addr.ToString() << std::endl
                  << std::flush;
        assert(0);
      }
    }
  }
}

void Remu::CheckLog() {}

void Remu::CheckMeta() {}

void Remu::CheckIndex() {}

void Remu::Create() {
  for (auto conf : configs) {
    auto sptr = loop.lock();
    assert(sptr);
    vraft::RaftServerSPtr ptr = std::make_shared<vraft::RaftServer>(sptr, conf);
    ptr->raft()->set_tracer_cb(tracer_cb);
    ptr->raft()->set_create_sm(create_sm);
    raft_servers.push_back(ptr);
  }
}

void Remu::Start() {
  for (auto &ptr : raft_servers) {
    if (ptr) {
      ptr->Start();
    }
  }
}

void Remu::Stop() {
  for (auto &ptr : raft_servers) {
    if (ptr) {
      ptr->Stop();
    }
  }
}

void Remu::Clear() {
  configs.clear();
  raft_servers.clear();
}

}  // namespace vraft

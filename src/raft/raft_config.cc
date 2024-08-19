#include "raft_config.h"

#include "common.h"

namespace vraft {

nlohmann::json RaftConfig::ToJson() {
  nlohmann::json j;
  j["me"][0] = me.ToU64();
  j["me"][1] = me.ToString();
  int32_t i = 0;
  for (auto peer : peers) {
    j["peers"][i][0] = peer.ToU64();
    j["peers"][i][1] = peer.ToString();
    i++;
  }
  return j;
}

nlohmann::json RaftConfig::ToJsonTiny() {
  nlohmann::json j;
  j["me"] = me.ToString();
  int32_t i = 0;
  for (auto peer : peers) {
    j["peers"][i++] = peer.ToString();
  }
  return j;
}

std::string RaftConfig::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["rc"] = ToJsonTiny();
  } else {
    j["raft_config"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

bool RaftConfig::InConfig(const RaftAddr &addr) {
  if (me.ToU64() == addr.ToU64()) {
    return true;
  }

  for (auto a : peers) {
    if (a.ToU64() == addr.ToU64()) {
      return true;
    }
  }

  return false;
}

}  // namespace vraft

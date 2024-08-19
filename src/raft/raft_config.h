#ifndef VRAFT_RAFT_CONFIG_H_
#define VRAFT_RAFT_CONFIG_H_

#include <vector>

#include "common.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"

namespace vraft {

struct RaftConfig {
  RaftAddr me;
  std::vector<RaftAddr> peers;

  int32_t MaxBytes();
  int32_t ToString(std::string& s);
  int32_t ToString(const char* ptr, int32_t len);
  int32_t FromString(std::string& s);
  int32_t FromString(const char* ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

using AppendConfigFunc = std::function<void(const RaftConfig& rc)>;
using DeleteConfigFunc = std::function<void(RaftIndex)>;

}  // namespace vraft

#endif

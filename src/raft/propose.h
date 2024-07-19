#ifndef VRAFT_PROPOSE_H_
#define VRAFT_PROPOSE_H_

#include <string>

#include "allocator.h"
#include "common.h"
#include "message.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"
#include "util.h"

namespace vraft {

struct Propose : public Message {
  uint32_t uid;
  std::string msg;

  int32_t MaxBytes() override;
  int32_t ToString(std::string &s) override;
  int32_t ToString(const char *ptr, int32_t len) override;
  int32_t FromString(std::string &s) override;
  int32_t FromString(const char *ptr, int32_t len) override;

  nlohmann::json ToJson() override;
  nlohmann::json ToJsonTiny() override;
  std::string ToJsonString(bool tiny, bool one_line) override;
};

inline int32_t Propose::MaxBytes() {
  int32_t sz = 0;
  sz += sizeof(uid);
  sz += 2 * sizeof(int32_t);
  sz += msg.size();
  return sz;
}

inline int32_t Propose::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr = reinterpret_cast<char *>(DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  DefaultAllocator().Free(ptr);
  return size;
}

inline int32_t Propose::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  EncodeFixed32(p, uid);
  p += sizeof(uid);
  size += sizeof(uid);

  Slice sls(msg.c_str(), msg.size());
  char *p2 = EncodeString2(p, len - size, sls);
  size += (p2 - p);
  p = p2;

  assert(size <= len);
  return size;
}

inline int32_t Propose::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

inline int32_t Propose::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  uid = DecodeFixed32(p);
  p += sizeof(uid);
  size += sizeof(uid);

  Slice result;
  Slice input(p, len - size);
  int32_t sz = DecodeString2(&input, &result);
  if (sz > 0) {
    msg.clear();
    msg.append(result.data(), result.size());
    size += sz;
  }

  return size;
}

inline nlohmann::json Propose::ToJson() {
  nlohmann::json j;
  j["uid"] = U32ToHexStr(uid);
  j["msg"] = StrToHexStr(msg.c_str(), msg.size());
  return j;
}

inline nlohmann::json Propose::ToJsonTiny() {
  nlohmann::json j;
  j["uid"] = U32ToHexStr(uid);
  j["msg"] = StrToHexStr(msg.c_str(), msg.size());
  return j;
}

inline std::string Propose::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["pps"] = ToJsonTiny();
  } else {
    j["propose"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft

#endif

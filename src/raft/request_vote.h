#ifndef VRAFT_REQUEST_VOTE_H_
#define VRAFT_REQUEST_VOTE_H_

#include <stdint.h>

#include "allocator.h"
#include "common.h"
#include "message.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"
#include "util.h"

namespace vraft {

struct RequestVote : public Message {
  RaftAddr src;   // uint64_t
  RaftAddr dest;  // uint64_t
  RaftTerm term;
  uint32_t uid;
  uint64_t send_ts;  // nanosecond
  uint64_t elapse;   // microsecond

  RaftIndex last_log_index;
  RaftTerm last_log_term;

  int32_t MaxBytes() override;
  int32_t ToString(std::string &s) override;
  int32_t ToString(const char *ptr, int32_t len) override;
  int32_t FromString(std::string &s) override;
  int32_t FromString(const char *ptr, int32_t len) override;

  nlohmann::json ToJson() override;
  nlohmann::json ToJsonTiny() override;
  std::string ToJsonString(bool tiny, bool one_line) override;
};

inline int32_t RequestVote::MaxBytes() {
  int32_t size = 0;
  size += sizeof(uint64_t);
  size += sizeof(uint64_t);
  size += sizeof(term);
  size += sizeof(uid);
  size += sizeof(send_ts);
  size += sizeof(elapse);
  size += sizeof(last_log_term);
  size += sizeof(last_log_index);
  return size;
}

inline int32_t RequestVote::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr = reinterpret_cast<char *>(DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  DefaultAllocator().Free(ptr);
  return size;
}

inline int32_t RequestVote::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;
  uint64_t u64 = 0;

  u64 = src.ToU64();
  EncodeFixed64(p, u64);
  p += sizeof(u64);
  size += sizeof(u64);

  u64 = dest.ToU64();
  EncodeFixed64(p, u64);
  p += sizeof(u64);
  size += sizeof(u64);

  EncodeFixed64(p, term);
  p += sizeof(term);
  size += sizeof(term);

  EncodeFixed32(p, uid);
  p += sizeof(uid);
  size += sizeof(uid);

  EncodeFixed64(p, send_ts);
  p += sizeof(send_ts);
  size += sizeof(send_ts);

  EncodeFixed64(p, elapse);
  p += sizeof(elapse);
  size += sizeof(elapse);

  EncodeFixed32(p, last_log_index);
  p += sizeof(last_log_index);
  size += sizeof(last_log_index);

  EncodeFixed64(p, last_log_term);
  p += sizeof(last_log_term);
  size += sizeof(last_log_term);

  assert(size <= len);
  return size;
}

inline int32_t RequestVote::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

inline int32_t RequestVote::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  uint64_t u64 = 0;
  int32_t size = 0;

  u64 = DecodeFixed64(p);
  src.FromU64(u64);
  p += sizeof(u64);
  size += sizeof(u64);

  u64 = DecodeFixed64(p);
  dest.FromU64(u64);
  p += sizeof(u64);
  size += sizeof(u64);

  term = DecodeFixed64(p);
  p += sizeof(term);
  size += sizeof(term);

  uid = DecodeFixed32(p);
  p += sizeof(uid);
  size += sizeof(uid);

  send_ts = DecodeFixed64(p);
  p += sizeof(send_ts);
  size += sizeof(send_ts);

  elapse = DecodeFixed64(p);
  p += sizeof(elapse);
  size += sizeof(elapse);

  last_log_index = DecodeFixed32(p);
  p += sizeof(last_log_index);
  size += sizeof(last_log_index);

  last_log_term = DecodeFixed64(p);
  p += sizeof(last_log_term);
  size += sizeof(last_log_term);

  return size;
}

inline nlohmann::json RequestVote::ToJson() {
  nlohmann::json j;
  j[0]["src"] = src.ToString();
  j[0]["dest"] = dest.ToString();
  j[0]["term"] = term;
  j[0]["uid"] = U32ToHexStr(uid);
  j[1]["last"] = last_log_index;
  j[1]["last-term"] = last_log_term;
  j[2]["send_ts"] = send_ts;
  j[2]["elapse"] = elapse;
  return j;
}

inline nlohmann::json RequestVote::ToJsonTiny() {
  nlohmann::json j;
  j["src"] = src.ToString();
  j["dst"] = dest.ToString();
  j["tm"] = term;
  j["last"] = last_log_index;
  j["ltm"] = last_log_term;
  j["uid"] = U32ToHexStr(uid);
  j["send"] = send_ts;
  j["elapse"] = elapse;
  return j;
}

inline std::string RequestVote::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["rv"] = ToJsonTiny();
  } else {
    j["request-vote"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft

#endif

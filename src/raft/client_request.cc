#include "client_request.h"

namespace vraft {

int32_t ClientRequest::MaxBytes() {
  int32_t sz = 0;
  sz += sizeof(uid);
  sz += sizeof(cmd);
  sz += 2 * sizeof(int32_t);
  sz += data.size();
  return sz;
}

int32_t ClientRequest::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr = reinterpret_cast<char *>(DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  DefaultAllocator().Free(ptr);
  return size;
}

int32_t ClientRequest::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  {
    EncodeFixed32(p, uid);
    size += sizeof(uid);
    p += sizeof(uid);
  }

  {
    EncodeFixed32(p, cmd);
    size += sizeof(cmd);
    p += sizeof(cmd);
  }

  {
    Slice sls(data.c_str(), data.size());
    char *p2 = EncodeString2(p, len - size, sls);
    size += (p2 - p);
    p = p2;
  }

  assert(size <= len);
  return size;
}

int32_t ClientRequest::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

int32_t ClientRequest::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  {
    uid = DecodeFixed32(p);
    p += sizeof(uid);
    size += sizeof(uid);
  }

  {
    cmd = DecodeFixed32(p);
    p += sizeof(cmd);
    size += sizeof(cmd);
  }

  {
    Slice result;
    Slice input(p, len - size);
    int32_t sz = DecodeString2(&input, &result);
    if (sz > 0) {
      data.clear();
      data.append(result.data(), result.size());
      size += sz;
    }
  }

  return size;
}

nlohmann::json ClientRequest::ToJson() {
  nlohmann::json j;
  j["uid"] = U32ToHexStr(uid);
  j["cmd"] = ClientCmdToStr(U32ToClientCmd(cmd));
  j["data"] = StrToHexStr(data.c_str(), data.size());
  return j;
}

nlohmann::json ClientRequest::ToJsonTiny() { return ToJson(); }

std::string ClientRequest::ToJsonString(bool tiny, bool one_line) {
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

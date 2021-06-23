#ifndef __VRAFT_CODING_H__
#define __VRAFT_CODING_H__

#include <string>
#include "vraft_rpc.pb.h"

namespace vraft {

void Term2String(int64_t term, std::string &buf);
bool String2Term(const std::string &buf, int64_t &term);


}  // namespace vraft

#endif

#include "coding.h"

namespace vraft {

void Term2String(int64_t term, std::string &buf) {
    vraft_rpc::Term pb;
    pb.set_term(term);
    bool ret = pb.SerializeToString(&buf);
    assert(ret);
}

bool String2Term(const std::string &buf, int64_t &term) {
    vraft_rpc::Term pb;
    bool ret = pb.ParseFromString(buf);
    if (ret) {
        term = pb.term();
    }
    return ret;
}

}  // namespace vraft

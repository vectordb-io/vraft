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

void
NodeId2String(uint64_t node_id, std::string &buf) {
    vraft_rpc::NodeId pb;
    pb.set_node_id(node_id);
    bool ret = pb.SerializeToString(&buf);
    assert(ret);
}

bool
String2NodeId(const std::string &buf, uint64_t &node_id) {
    vraft_rpc::NodeId pb;
    bool ret = pb.ParseFromString(buf);
    if (ret) {
        node_id = pb.node_id();
    }
    return ret;
}

jsonxx::json64
ToJson(const vraft_rpc::RequestVote &pb) {
    jsonxx::json64 j, jret;
    j["term"] = pb.term();
    j["node_id"] = pb.node_id();
    j["last_log_index"] = pb.last_log_index();
    j["last_log_term"] = pb.last_log_term();
    jret["RequestVote"] = j;
    return jret;
}

std::string
ToString(const vraft_rpc::RequestVote &pb) {
    return ToJson(pb).dump();
}

std::string
ToStringPretty(const vraft_rpc::RequestVote &pb) {
    return ToJson(pb).dump(4, ' ');
}

jsonxx::json64
ToJson(const vraft_rpc::RequestVoteReply &pb) {
    jsonxx::json64 j, jret;
    j["term"] = pb.term();
    j["vote_granted"] = pb.vote_granted();
    j["node_id"] = pb.node_id();
    jret["RequestVoteReply"] = j;
    return jret;
}

std::string
ToString(const vraft_rpc::RequestVoteReply &pb) {
    return ToJson(pb).dump();
}

std::string
ToStringPretty(const vraft_rpc::RequestVoteReply &pb) {
    return ToJson(pb).dump(4, ' ');
}

jsonxx::json64
ToJson(const vraft_rpc::AppendEntries &pb) {
    jsonxx::json64 j, jret;
    j["term"] = pb.term();
    j["node_id"] = pb.node_id();
    j["prev_log_index"] = pb.prev_log_index();
    j["prev_log_term"] = pb.prev_log_term();
    // Entries
    j["commit_index"] = pb.commit_index();
    jret["AppendEntries"] = j;
    return jret;
}

std::string
ToString(const vraft_rpc::AppendEntries &pb) {
    return ToJson(pb).dump();
}

std::string
ToStringPretty(const vraft_rpc::AppendEntries &pb) {
    return ToJson(pb).dump(4, ' ');
}

jsonxx::json64
ToJson(const vraft_rpc::AppendEntriesReply &pb) {
    jsonxx::json64 j, jret;
    j["term"] = pb.term();
    j["success"] = pb.success();
    jret["AppendEntriesReply"] = j;
    return jret;

}

std::string
ToString(const vraft_rpc::AppendEntriesReply &pb) {
    return ToJson(pb).dump();
}

std::string
ToStringPretty(const vraft_rpc::AppendEntriesReply &pb) {
    return ToJson(pb).dump(4, ' ');
}

}  // namespace vraft

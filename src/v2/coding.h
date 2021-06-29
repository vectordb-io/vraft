#ifndef __VRAFT_CODING_H__
#define __VRAFT_CODING_H__

#include <string>
#include "jsonxx/json.hpp"
#include "vraft_rpc.pb.h"

namespace vraft {

void Term2String(int64_t term, std::string &buf);
bool String2Term(const std::string &buf, int64_t &term);

void NodeId2String(uint64_t node_id, std::string &buf);
bool String2NodeId(const std::string &buf, uint64_t &node_id);

jsonxx::json64 ToJson(const vraft_rpc::RequestVote &pb);
std::string ToString(const vraft_rpc::RequestVote &pb);
std::string ToStringPretty(const vraft_rpc::RequestVote &pb);

jsonxx::json64 ToJson(const vraft_rpc::RequestVoteReply &pb);
std::string ToString(const vraft_rpc::RequestVoteReply &pb);
std::string ToStringPretty(const vraft_rpc::RequestVoteReply &pb);

jsonxx::json64 ToJson(const vraft_rpc::AppendEntries &pb);
std::string ToString(const vraft_rpc::AppendEntries &pb);
std::string ToStringPretty(const vraft_rpc::AppendEntries &pb);

jsonxx::json64 ToJson(const vraft_rpc::AppendEntriesReply &pb);
std::string ToString(const vraft_rpc::AppendEntriesReply &pb);
std::string ToStringPretty(const vraft_rpc::AppendEntriesReply &pb);

}  // namespace vraft

#endif

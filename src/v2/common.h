#ifndef __VRAFT_COMMON_H__
#define __VRAFT_COMMON_H__

#include "status.h"
#include "vraft_rpc.grpc.pb.h"

namespace vraft {

using OnPingCallBack = std::function<void (const vraft_rpc::Ping &request, vraft_rpc::PingReply &reply)>;
using PingFinishCallBack = std::function<Status (vraft_rpc::PingReply)>;

using OnClientRequestCallBack = std::function<void (const vraft_rpc::ClientRequest &request, void *async_flag)>;

using OnRequestVoteCallBack = std::function<void (const vraft_rpc::RequestVote &request, vraft_rpc::RequestVoteReply &reply)>;
using RequestVoteFinishCallBack = std::function<Status (vraft_rpc::RequestVoteReply)>;

using OnAppendEntriesCallBack = std::function<void (const vraft_rpc::AppendEntries &request, vraft_rpc::AppendEntriesReply &reply)>;
using AppendEntriesFinishCallBack = std::function<Status (vraft_rpc::AppendEntriesReply)>;


}  // namespace vraft

#endif

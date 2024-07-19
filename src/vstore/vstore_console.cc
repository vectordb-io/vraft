#include "vstore_console.h"

#include "propose.h"
#include "util.h"
#include "vstore_msg.h"

namespace vstore {

int32_t VstoreConsole::Parse(const std::string &cmd_line) {
  Clear();

  std::string cmd_line2 = cmd_line;
  vraft::DelHead(cmd_line2, " ");
  vraft::DelTail(cmd_line2, " ;");

  std::vector<std::string> result;
  vraft::Split(cmd_line2, ' ', result);

  cmd_ = result[0];

  if (cmd_ == "get" && result.size() == 2) {
    key_ = result[1];
  }

  if (cmd_ == "set" && result.size() == 3) {
    key_ = result[1];
    value_ = result[2];
  }

  return 0;
}

int32_t VstoreConsole::Execute() {
  if (cmd_ == "set") {
    vraft::Propose msg;
    msg.uid = UniqId(&msg);
    msg.msg = key_ + ":" + value_;

    std::string body_str;
    int32_t bytes = msg.ToString(body_str);

    vraft::MsgHeader header;
    header.body_bytes = bytes;
    header.type = vraft::kPropose;
    std::string header_str;
    header.ToString(header_str);
    header_str.append(std::move(body_str));
    Send(header_str);

    set_result("ok");
    ResultReady();

  } else if (cmd_ == "get") {
    VstoreGet msg;
    msg.uid = UniqId(&msg);
    msg.key = key_;

    std::string body_str;
    int32_t bytes = msg.ToString(body_str);

    vraft::MsgHeader header;
    header.body_bytes = bytes;
    header.type = vraft::kVstoreGet;
    std::string header_str;
    header.ToString(header_str);
    header_str.append(std::move(body_str));
    Send(header_str);
  }

  return 0;
}

void VstoreConsole::OnMessage(const vraft::TcpConnectionSPtr &conn,
                              vraft::Buffer *buf) {
  set_result(std::string(buf->Peek()));
  buf->RetrieveAll();
  ResultReady();
}

void VstoreConsole::Clear() {
  cmd_.clear();
  key_.clear();
  value_.clear();
}

}  // namespace vstore

#include <algorithm>
#include <cstdlib>
#include <fstream>

#include "clock.h"
#include "raft.h"
#include "raft_server.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

int32_t Raft::OnPropose(struct Propose &msg, vraft::TcpConnectionSPtr conn) {
  int32_t rv = 0;
  if (started_) {
    vraft_logger.Info("%s recv propose msg:%s", Me().ToString().c_str(),
                      msg.ToJsonString(true, true).c_str());

    Tracer tracer(this, false, tracer_cb_);
    tracer.PrepareState0();
    tracer.PrepareEvent(kEventRecv, msg.ToJsonString(false, true));

    rv = Propose(msg.msg, nullptr);

    tracer.PrepareState1();
    tracer.Finish();
  }

  return rv;
}

}  // namespace vraft

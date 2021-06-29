#ifndef __VRAFT_ASYNC_REQ_MANAGER_H__
#define __VRAFT_ASYNC_REQ_MANAGER_H__

#include <cassert>
#include <set>
#include <mutex>
#include <thread>
#include <memory>
#include <glog/logging.h>
#include "status.h"

namespace vraft {

class AsyncReqManager {
  public:
    AsyncReqManager() = default;
    AsyncReqManager(const AsyncReqManager&) = delete;
    AsyncReqManager& operator=(const AsyncReqManager&) = delete;
    ~AsyncReqManager() = default;

    void Add(void *p);
    void Delete(void *p);
    bool Has(void *p) const;
    Status Init();

  private:
    std::set<void*> ptrs_;
    mutable std::mutex mutex_;
};

} // namespace vraft

#endif

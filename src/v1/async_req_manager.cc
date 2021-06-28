#include "async_req_manager.h"

namespace vraft {

Status
AsyncReqManager::Init() {
    std::unique_lock<std::mutex> guard(mutex_);
    ptrs_.clear();
    return Status::OK();
}

void
AsyncReqManager::Add(void *p) {
    std::unique_lock<std::mutex> guard(mutex_);
    auto it = ptrs_.find(p);
    assert(it == ptrs_.end());
    ptrs_.insert(p);
}

void
AsyncReqManager::Delete(void *p) {
    std::unique_lock<std::mutex> guard(mutex_);
    auto it = ptrs_.find(p);
    assert(it != ptrs_.end());
    ptrs_.erase(it);
}

bool
AsyncReqManager::Has(void *p) const {
    std::unique_lock<std::mutex> guard(mutex_);
    auto it = ptrs_.find(p);
    return  (it != ptrs_.end());
}

} // namespace vraft

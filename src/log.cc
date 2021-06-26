#include "log.h"

namespace vraft {

Log::Log(const std::string &s) {
}

Log::~Log() {
}

Status
Log::Init() {
    return Status::OK();
}

}  // namespace vraft

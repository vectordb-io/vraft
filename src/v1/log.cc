#include "log.h"

namespace vraft {

Log::Log(const std::string &path)
    :path_(path) {
}

Log::~Log() {
}

Status
Log::Init() {
    return Status::OK();
}

}  // namespace vraft

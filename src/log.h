#ifndef __VRAFT_LOG_H__
#define __VRAFT_LOG_H__

#include <cassert>
#include <string>
#include <leveldb/db.h>
#include "status.h"

namespace vraft {

class LogId {
  public:
    LogId()
        :index_(0),
         term_(0) {
    }

    LogId(int64_t index, int64_t term)
        :index_(index),
         term_(term) {
    }

    int64_t index() const {
        return index_;
    }

    int64_t term() const {
        return term_;
    }

  private:
    int64_t index_;
    int64_t term_;
};

class Log {
  public:
    Log(const std::string &path);
    ~Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    Status Init();

    LogId LastLogId() const {
        return LogId();
    }

  private:
    std::string path_;
    leveldb::DB* db_;
};

}  // namespace vraft

#endif

#ifndef __VRAFT_LOG_H__
#define __VRAFT_LOG_H__

#include <cassert>
#include <mutex>
#include <string>
#include "jsonxx/json.hpp"
#include <leveldb/db.h>
#include "vraft_rpc.pb.h"
#include "status.h"

namespace vraft {

class Entry {
  public:
    Entry()
        :term_(0) {
    }

    Entry(int64_t term, const std::string &cmd)
        :term_(term), cmd_(cmd) {
    }

    ~Entry() = default;
    Entry(const Entry&) = default;
    Entry& operator=(Entry&) = default;

    bool SerializeToString(std::string &s) const;
    bool ParseFromString(const std::string &s);

    int64_t term() const {
        return term_;
    }

    void set_term(int64_t term) {
        term_ = term;
    }

    const std::string& cmd() const {
        return cmd_;
    }

    void set_cmd(const std::string &cmd) {
        cmd_ = cmd;
    }

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
    int64_t term_;
    std::string cmd_;
};

class Log {
  public:
    Log(const std::string &path);
    ~Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    Status Init();

    int LastLogIndex() const;
    int LastLogTerm() const;
    Status GetEntry(int index, Entry &entry) const;
    Status AppendEntry(const Entry &entry);
    Status TruncateEntries(int from_index);
    int Len() const;

    jsonxx::json64 ToJson() const;
    std::string ToString() const;
    std::string ToStringPretty() const;

  private:
#define KEY_LAST_LOG_INDEX std::string("KEY_LAST_LOG_INDEX")

  private:
    Status DeleteLastEntry();

  private:
    mutable std::mutex mutex_;

    int last_log_index_;  // log index begin from 1
    std::string path_;
    leveldb::DB* db_;
};

}  // namespace vraft

#endif

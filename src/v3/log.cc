#include <glog/logging.h>
#include "leveldb/write_batch.h"
#include "log.h"

namespace vraft {

// ---------------------------------------------------------------------------------------------
// class Entry
bool
Entry::SerializeToString(std::string &s) const {
    vraft_rpc::Entry pb;
    pb.set_term(term_);
    pb.set_cmd(cmd_);
    bool b = pb.SerializeToString(&s);
    return b;
}

bool
Entry::ParseFromString(const std::string &s) {
    vraft_rpc::Entry pb;
    bool b = pb.ParseFromString(s);
    if (b) {
        term_ = pb.term();
        cmd_ = pb.cmd();
    }
    return b;
}

jsonxx::json64
Entry::ToJson() const {
    jsonxx::json64 j, jret;
    j["term"] = term_;
    j["cmd"] = cmd_;
    jret["Entry"] = j;
    return jret;
}

std::string
Entry::ToString() const {
    return ToJson().dump();
}

std::string
Entry::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}

// ---------------------------------------------------------------------------------------------
// class Log
Log::Log(const std::string &path)
    :path_(path),
     db_(nullptr) {
}

Log::~Log() {
    char buf[128];
    snprintf(buf, sizeof(buf), "delete db ptr: %p", db_);
    LOG(INFO) << buf;
    delete db_;
}

Status
Log::Init() {
    std::unique_lock<std::mutex> guard(mutex_);

    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status s = leveldb::DB::Open(options, path_, &db_);
    assert(s.ok());

    std::string value;
    vraft_rpc::LogIndex i;
    s = db_->Get(leveldb::ReadOptions(), KEY_LAST_LOG_INDEX, &value);
    if (s.IsNotFound()) {
        last_log_index_ = 0;
        i.set_index(last_log_index_);
        auto b = i.SerializeToString(&value);
        assert(b);
        leveldb::WriteOptions wo;
        wo.sync = true;
        s = db_->Put(wo, KEY_LAST_LOG_INDEX, value);
        assert(s.ok());

    } else {
        assert(s.ok());
        auto b = i.ParseFromString(value);
        assert(b);
        last_log_index_ = i.index();
    }

    return Status::OK();
}

int
Log::LastLogIndex() const {
    std::unique_lock<std::mutex> guard(mutex_);
    return last_log_index_;
}

int
Log::LastLogTerm() const {
    std::unique_lock<std::mutex> guard(mutex_);

    if (last_log_index_ == 0) {
        return 0;
    }

    Entry entry;
    auto s = GetEntry(last_log_index_, entry);
    assert(s.ok());

    return entry.term();
}

Status
Log::GetEntry(int index, Entry &entry) const {
    // do not lock!

    std::string index_str;
    vraft_rpc::LogIndex i;
    i.set_index(index);
    auto b = i.SerializeToString(&index_str);
    assert(b);

    std::string entry_str;
    auto s = db_->Get(leveldb::ReadOptions(), index_str, &entry_str);
    if (s.IsNotFound()) {
        return Status::NotFound("log entry");
    }
    assert(s.ok());
    b = entry.ParseFromString(entry_str);
    assert(b);
    return Status::OK();
}

Status
Log::AppendEntry(const Entry &entry) {
    std::unique_lock<std::mutex> guard(mutex_);
    //LOG(INFO) << "AppendEntry: " << entry.ToString();

    last_log_index_++;
    std::string index_str;
    vraft_rpc::LogIndex i;
    i.set_index(last_log_index_);
    auto b = i.SerializeToString(&index_str);
    assert(b);

    std::string entry_str;
    b = entry.SerializeToString(entry_str);
    assert(b);

    leveldb::WriteBatch batch;
    batch.Put(KEY_LAST_LOG_INDEX, index_str);
    batch.Put(index_str, entry_str);
    leveldb::WriteOptions wo;
    wo.sync = true;
    auto s = db_->Write(wo, &batch);
    assert(s.ok());

    return Status::OK();
}

Status
Log::TruncateEntries(int from_index) {
    std::unique_lock<std::mutex> guard(mutex_);
    if (from_index > 0) {
        for (int i = last_log_index_; i >= from_index; i--) {
            auto s = DeleteLastEntry();
            assert(s.ok());
        }
    }
    return Status::OK();
}

int
Log::Len() const {
    std::unique_lock<std::mutex> guard(mutex_);
    return last_log_index_;
}

jsonxx::json64
Log::ToJson() const {
    jsonxx::json64 j, jret;
    j["last_log_index"] = LastLogIndex();
    j["last_log_term"] = LastLogTerm();
    j["size"] = Len();
    for (int i = 1; i <= last_log_index_; ++i) {
        Entry entry;
        auto s = GetEntry(i, entry);
        assert(s.ok());

        char buf[128];
        snprintf(buf, sizeof(buf), "%d", i);
        j["entries"][std::string(buf)] = entry.ToJson();
    }
    jret["Log"] = j;
    return jret;
}

std::string
Log::ToString() const {
    return ToJson().dump();
}

std::string
Log::ToStringPretty() const {
    return ToJson().dump(4, ' ');
}

Status
Log::DeleteLastEntry() {
    // do not lock! be careful dead lock!

    if (last_log_index_ > 0) {
        std::string index_str;
        vraft_rpc::LogIndex i;
        i.set_index(last_log_index_);
        auto b = i.SerializeToString(&index_str);
        assert(b);

        leveldb::WriteBatch batch;
        batch.Put(KEY_LAST_LOG_INDEX, index_str);
        batch.Delete(index_str);
        leveldb::WriteOptions wo;
        wo.sync = true;
        auto s = db_->Write(wo, &batch);
        assert(s.ok());
        last_log_index_--;
    }
    return Status::OK();
}

}  // namespace vraft

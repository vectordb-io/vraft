#include "node.h"
#include "coding.h"
#include "config.h"
#include "storage.h"

namespace vraft {

Storage::~Storage() {
    delete db_;
}

Storage::Storage(const std::string &s)
    :path_(s) {
}

Status
Storage::CurrentTerm(int64_t &term) const {
    std::string buf;
    auto s = db_->Get(leveldb::ReadOptions(), KEY_CURRENT_TERM, &buf);
    if (s.IsNotFound()) {
        return Status::NotFound(KEY_CURRENT_TERM);
    }
    auto ret = String2Term(buf, term);
    assert(ret);
    return Status::OK();
}

Status
Storage::CurrentTermPersist(int64_t term) {
    std::string buf;
    Term2String(term, buf);

    leveldb::WriteOptions wo;
    wo.sync = true;
    auto s = db_->Put(wo, KEY_CURRENT_TERM, buf);
    assert(s.ok());
    return Status::OK();
}

Status
Storage::VoteFor(std::string &vote_for) const {
    auto s = db_->Get(leveldb::ReadOptions(), KEY_VOTE_FOR, &vote_for);
    if (s.IsNotFound()) {
        return Status::NotFound(KEY_CURRENT_TERM);
    }
    return Status::OK();
}

Status
Storage::VoteForPersist(const std::string &vote_for) {
    leveldb::WriteOptions wo;
    wo.sync = true;
    auto s = db_->Put(wo, KEY_VOTE_FOR, vote_for);
    assert(s.ok());
    return Status::OK();
}

Status
Storage::Init() {
    Status s;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, path_, &db_);
    assert(status.ok());

    return Status::OK();
}


}  // namespace vraft

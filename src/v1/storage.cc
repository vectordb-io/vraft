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
Storage::PersistCurrentTerm(int64_t term) {
    std::string buf;
    Term2String(term, buf);

    leveldb::WriteOptions wo;
    wo.sync = true;
    auto s = db_->Put(wo, KEY_CURRENT_TERM, buf);
    assert(s.ok());
    return Status::OK();
}

Status
Storage::VoteFor(uint64_t &node_id) const {
    std::string buf;
    auto s = db_->Get(leveldb::ReadOptions(), KEY_VOTE_FOR, &buf);
    if (s.IsNotFound()) {
        return Status::NotFound(KEY_CURRENT_TERM);
    }
    auto ret = String2NodeId(buf, node_id);
    assert(ret);
    return Status::OK();
}

Status
Storage::PersistVoteFor(uint64_t node_id) {
    std::string buf;
    NodeId2String(node_id, buf);

    leveldb::WriteOptions wo;
    wo.sync = true;
    auto s = db_->Put(wo, KEY_VOTE_FOR, buf);
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

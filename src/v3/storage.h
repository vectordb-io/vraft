#ifndef __VRAFT_STORAGE_H__
#define __VRAFT_STORAGE_H__

#include <cassert>
#include <string>
#include <leveldb/db.h>
#include "status.h"

namespace vraft {

class Storage {
  public:

#define KEY_CURRENT_TERM std::string("KEY_CURRENT_TERM")
#define KEY_VOTE_FOR std::string("KEY_VOTE_FOR")

    Storage(const std::string &s);
    ~Storage();
    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

    Status Init();

    Status CurrentTerm(int64_t &term) const;
    Status PersistCurrentTerm(int64_t term);
    Status VoteFor(uint64_t &node_id) const;
    Status PersistVoteFor(uint64_t node_id);

  private:
    std::string path_;
    leveldb::DB* db_;
};

}  // namespace vraft

#endif

#ifndef __VRAFT_BALLOT_H__
#define __VRAFT_BALLOT_H__

#include <stdint.h>

namespace vraft {

class Ballot {
  public:
    Ballot()
        :proposal_id_(0),
         node_id_(0) {
    }

    Ballot(uint64_t node_id)
        :proposal_id_(0),
         node_id_(node_id) {
    }

    Ballot(uint64_t proposal_id, uint64_t node_id)
        :proposal_id_(proposal_id),
         node_id_(node_id) {
    }

    ~Ballot() = default;
    Ballot(const Ballot&) = default;
    Ballot& operator=(const Ballot&) = default;

    void Init(uint64_t proposal_id, uint64_t node_id) {
        proposal_id_ = proposal_id;
        node_id_ = node_id;
    }

    void Clear() {
        proposal_id_ = 0;
        node_id_ = 0;
    }

    bool IsNull() const {
        return (proposal_id_ == 0 && node_id_ == 0);
    }

    std::string ToString() const {
        char buf[128];
        snprintf(buf, sizeof(buf), "node_id:%lu, proposal_id:%lu", node_id_, proposal_id_);
        return std::string(buf);
    }

    // ballot++, not ++ballot
    void operator++(int arg) {
        proposal_id_++;
    }

    bool operator!=(const Ballot &rhs) const {
        return (proposal_id_ != rhs.proposal_id_
                || node_id_ != rhs.node_id_);
    }

    bool operator==(const Ballot &rhs) const {
        return (proposal_id_ == rhs.proposal_id_
                && node_id_ == rhs.node_id_);
    }

    bool operator>(const Ballot &rhs) const {
        if (proposal_id_ == rhs.proposal_id_) {
            return node_id_ > rhs.node_id_;
        } else {
            return proposal_id_ > rhs.proposal_id_;
        }
    }

    bool operator>=(const Ballot &rhs) const {
        if (proposal_id_ == rhs.proposal_id_) {
            return node_id_ >= rhs.node_id_;
        } else {
            return proposal_id_ >= rhs.proposal_id_;
        }
    }

    void set_proposal_id(uint64_t proposal_id) {
        proposal_id_ = proposal_id;
    }

    uint64_t proposal_id() const {
        return proposal_id_;
    }

    void set_node_id(uint64_t node_id) {
        node_id_ = node_id;
    }

    uint64_t node_id() const {
        return node_id_;
    }

  private:
    uint64_t proposal_id_;
    uint64_t node_id_;
};

}  // namespace vraft

#endif

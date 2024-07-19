#ifndef VSTORE_TPL_H_
#define VSTORE_TPL_H_

#include <memory>

namespace vstore {

class Tpl;
using TplSPtr = std::shared_ptr<Tpl>;
using TplUPtr = std::unique_ptr<Tpl>;
using TplWPtr = std::weak_ptr<Tpl>;

struct Tpl {

};

} // namespace vstore

#endif

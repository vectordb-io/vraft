#include "util.h"

namespace vraft {
namespace util {

static std::default_random_engine random_(time(nullptr));

int random_int(int min, int max) {
    std::uniform_int_distribution<int> random_range(min, max);
    int n = random_range(random_);
    return n;
}


}  // namespace util
}  // namespace vraft

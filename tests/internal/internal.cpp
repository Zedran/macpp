#include "internal.hpp"

namespace internal {

void print_vector(std::ostringstream& oss, const std::vector<int64_t>& vec) {
    for (size_t i = 0; i < vec.size(); i++)
        oss << vec[i] << (i != vec.size() - 1 ? " " : "");
}

} // namespace internal

#include <sstream>
#include <vector>

namespace internal {

void append_vector(std::ostringstream& oss, const std::vector<int64_t>& vec) {
    for (size_t i = 0; i < vec.size(); i++)
        oss << vec[i] << (i != vec.size() - 1 ? " " : "");
}

} // namespace internal

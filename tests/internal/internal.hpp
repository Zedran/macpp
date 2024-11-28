#pragma once

#include <sstream>
#include <vector>

namespace internal {

// A helper function that directs a space-separated list of vec elements to oss.
void print_vector(std::ostringstream& oss, const std::vector<int64_t>& vec);

} // namespace internal

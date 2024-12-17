#pragma once

#include <cstdint>
#include <sqlite3.h>
#include <sstream>
#include <vector>

namespace internal {

// Opens an in-memory database with empty vendors table for testing cache.
// Returns sqlite3 code.
int open_mem_db(sqlite3*& conn);

// A helper function that directs a space-separated list of vec elements to oss.
void print_vector(std::ostringstream& oss, const std::vector<int64_t>& vec);

} // namespace internal

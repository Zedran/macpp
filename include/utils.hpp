#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// A helper function that appends the correct number of placeholders
// to the sqlite statement in construction.
std::string build_find_by_addr_stmt(const size_t length) noexcept;

// Constructs a vector of all possible vendor identifiers that can be extracted
// from addr. This is important in situations where user specifies
// a full MAC address. Lookup by prefix is done by integer comparison.
// Including device identifier in the converted integer would result in missed
// searches. It is also important to trim address to the length
// of the different blocks to get match.
std::vector<int64_t> construct_queries(const std::string& addr);

// Returns a vendor identifier block of length block_len extracted from addr.
// If length of addr is lower than block_len, nullopt is returned instead.
// Accepts addr without separators (101010).
std::optional<std::string> get_ieee_block(const std::string& addr, const size_t block_len);

// Converts MAC prefix from string to an integer. Colon separators allowed.
int64_t prefix_to_int(const std::string& prefix);

// Converts integral MAC prefix to colon-separated string.
std::string prefix_to_string(const int64_t prefix);

// Removes ':' characters from addr.
std::string remove_addr_separators(const std::string& addr);

// Replaces "" (CSV escaped quotes) in str with ".
void replace_escaped_quotes(std::string& str);

// Inserts an escape character before every occurence of '%' and '_' in str.
std::string suppress_like_wildcards(const std::string& str) noexcept;

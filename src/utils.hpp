#pragma once

#include <cstdint>
#include <string>
#include <vector>

template <class F>
struct FinalAction {
    F act;

    explicit FinalAction(F f) : act(f) {}
    ~FinalAction() { act(); };
};

template <class F>
[[nodiscard]] auto finally(F f) {
    return FinalAction{f};
}

// A helper function that appends the correct number of placeholders
// to the sqlite statement in construction.
std::string build_query_by_id_stmt(const size_t length);

// Constructs a vector of all possible vendor identifiers that can be extracted
// from addr. This is important in situations where user specifies
// a full MAC address. Lookup by prefix is done by integer comparison.
// Including device identifier in the converted integer would result in missed
// searches. It is also important to trim address to the length
// of the different blocks to get match.
std::vector<int64_t> construct_queries(const std::string& addr);

// Returns a vendor identifier block of length block_len extracted from addr.
// If length of addr is lower than block_len, empty string is returned.
std::string get_ieee_block(const std::string& addr, const size_t block_len);

// Converts MAC prefix (10:10:10) from string to an integer.
int64_t prefix_to_id(const std::string& prefix);

// Removes "" (CSV escaped quotes) from str.
void replace_escaped_quotes(std::string& str);

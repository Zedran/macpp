#include <algorithm>
#include <sstream>

#include "AppError.hpp"
#include "utils.hpp"

std::string build_query_by_id_stmt(const size_t length) {
    std::ostringstream stmt_stream;
    stmt_stream << "SELECT * FROM vendors WHERE id IN (?";

    // From 1, the first placeholder is appended beforehand.
    for (size_t i = 1; i < length; i++) {
        stmt_stream << ",?";
    }
    stmt_stream << ");";

    return stmt_stream.str();
}

std::vector<int64_t> construct_queries(const std::string& addr) {
    constexpr size_t VENDOR_BLOCK_LENGTHS[3] = {6, 7, 9};

    std::vector<int64_t> queries;
    queries.reserve(3);

    std::string ieee_block;
    for (const auto& block_len : VENDOR_BLOCK_LENGTHS) {
        ieee_block = get_ieee_block(addr, block_len);

        if (!ieee_block.empty()) {
            queries.push_back(prefix_to_id(ieee_block));
        }
    }

    if (queries.empty()) {
        throw AppError("query '" + addr + "' too short");
    }

    return queries;
}

std::string get_ieee_block(const std::string& addr, const size_t block_len) {
    std::string stripped = addr;

    stripped.erase(std::remove(stripped.begin(), stripped.end(), ':'), stripped.end());

    if (stripped.length() < block_len) {
        return "";
    }

    stripped = stripped.substr(0, block_len);

    return stripped;
}

int64_t prefix_to_id(const std::string& prefix) {
    std::string s = prefix;
    s.erase(std::remove(s.begin(), s.end(), ':'), s.end());

    size_t  pos  = 0;
    int64_t conv = std::stoll(s, &pos, 16);

    // Check if stoll did not stop too early
    if (pos != s.length())
        throw(std::invalid_argument("non-hex character encountered"));

    return conv;
}

void replace_escaped_quotes(std::string& str) {
    const std::string target      = "\"\"";
    const std::string replacement = "\"";

    size_t pos = 0;

    while ((pos = str.find(target, pos)) != std::string::npos) {
        str.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }
}

std::string suppress_like_wildcards(const std::string& str) {
    const char PERCENT = '%';
    const char USCORE  = '_';

    if (str.find(PERCENT) == std::string::npos && str.find(USCORE) == std::string::npos) {
        return str;
    }

    std::ostringstream buffer;

    for (const auto& c : str) {
        if (c == PERCENT || c == USCORE)
            buffer << '\\';
        buffer << c;
    }

    return buffer.str();
}

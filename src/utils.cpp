#include "exception.hpp"
#include "utils.hpp"

std::string build_query_by_id_stmt(const size_t length) {
    std::string stmt = "SELECT * FROM vendors WHERE id IN (?";

    // Start from 1, since the first placeholder is appended beforehand.
    for (size_t i = 1; i < length; i++) {
        stmt += ",?";
    }
    stmt += ");";

    return stmt;
}

std::vector<int64_t> construct_queries(const std::string& addr) {
    constexpr size_t VENDOR_BLOCK_LENGTHS[3] = {6, 7, 9};

    std::vector<int64_t> queries;
    queries.reserve(3);

    std::optional<std::string> ieee_block;

    for (const auto& block_len : VENDOR_BLOCK_LENGTHS) {
        ieee_block = get_ieee_block(addr, block_len);

        if (ieee_block) {
            queries.push_back(prefix_to_id(*ieee_block));
        }
    }

    if (queries.empty()) {
        throw errors::AddrTooShortError;
    }

    return queries;
}

std::optional<std::string> get_ieee_block(const std::string& addr, const size_t block_len) {
    if (addr.length() >= block_len) {
        return addr.substr(0, block_len);
    }
    return std::nullopt;
}

int64_t prefix_to_id(const std::string& prefix) {
    size_t  pos  = 0;
    int64_t conv = std::stoll(prefix, &pos, 16);

    // Check if stoll did not stop too early
    if (pos != prefix.length()) {
        throw errors::AddrInvalidError;
    }
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
    constexpr char PERCENT = '%';
    constexpr char USCORE  = '_';

    if (str.find(PERCENT) == std::string::npos && str.find(USCORE) == std::string::npos) {
        return str;
    }

    std::string suppressed{};

    for (const auto& c : str) {
        if (c == PERCENT || c == USCORE)
            suppressed += '\\';
        suppressed += c;
    }

    return suppressed;
}

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "exception.hpp"
#include "utils.hpp"

std::string build_find_by_addr_stmt(const size_t length) noexcept {
    std::string stmt = "SELECT * FROM vendors WHERE prefix IN (?";

    // Start from 1, since the first placeholder is appended beforehand.
    for (size_t i = 1; i < length; i++) {
        stmt += ",?";
    }
    stmt += ')';

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
            queries.push_back(prefix_to_int(*ieee_block));
        }
    }

    if (queries.empty()) {
        throw errors::Error{"specified MAC address is too short"};
    }

    return queries;
}

std::optional<std::string> get_ieee_block(const std::string& addr, const size_t block_len) {
    if (addr.length() >= block_len) {
        return addr.substr(0, block_len);
    }
    return std::nullopt;
}

int64_t prefix_to_int(const std::string& prefix) {
    std::string clean = remove_addr_separators(prefix);

    size_t  pos = 0;
    int64_t conv;

    try {
        conv = std::stoll(clean, &pos, 16);
    } catch (const std::invalid_argument&) {
        throw errors::Error{"specified MAC address contains invalid characters"};
    }

    // Check if stoll did not stop too early
    if (pos != clean.length()) {
        throw errors::Error{"specified MAC address contains invalid characters"};
    }

    if (conv < 0) {
        throw errors::Error{"specified MAC address is negative"};
    }

    return conv;
}

std::string prefix_to_string(const int64_t prefix) {
    static constexpr size_t MIN_PREFIX_LEN = 6;

    std::ostringstream ss;
    ss << std::uppercase << std::hex << prefix;

    std::string hex_str = ss.str();

    ss.str("");
    ss.clear();

    if (hex_str.size() < MIN_PREFIX_LEN) {
        hex_str.insert(0, MIN_PREFIX_LEN - hex_str.size(), '0');
    }

    for (size_t i = 0; i < hex_str.size(); i++) {
        ss << hex_str[i];
        if (i < hex_str.size() - 1 && i % 2 == 1) {
            ss << ':';
        }
    }

    return ss.str();
}

std::string remove_addr_separators(const std::string& addr) {
    std::string stripped = addr;
    stripped.erase(std::remove(stripped.begin(), stripped.end(), ':'), stripped.end());
    return stripped;
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

std::string suppress_like_wildcards(const std::string& str) noexcept {
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

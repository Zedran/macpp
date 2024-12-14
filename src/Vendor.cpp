#include <sqlite3.h>
#include <sstream>
#include <string>

#include "Vendor.hpp"
#include "utils.hpp"

static inline std::string get_column_text(sqlite3_stmt* stmt, int coln);

Vendor::Vendor(const std::string& line) {
    std::string        priv_str;
    std::istringstream line_stream(line);

    std::getline(line_stream, mac_prefix, ',');

    size_t pos;
    if ((pos = line.find("\",")) != std::string::npos) {
        const size_t length = pos - mac_prefix.length() - 2;

        line_stream.seekg(1, std::ios::cur);

        vendor_name.resize(length);
        line_stream.read(&vendor_name[0], length);
        replace_escaped_quotes(vendor_name);

        line_stream.seekg(2, std::ios::cur);
    } else {
        std::getline(line_stream, vendor_name, ',');
    }

    std::getline(line_stream, priv_str, ',');
    std::getline(line_stream, block_type, ',');
    std::getline(line_stream, last_update);

    is_private = (priv_str == "true") ? true : false;
}

Vendor::Vendor(
    const std::string& mac_prefix,
    const std::string& vendor_name,
    const bool         is_private,
    const std::string& block_type,
    const std::string& last_update
) : mac_prefix(mac_prefix),
    vendor_name(vendor_name),
    is_private(is_private),
    block_type(block_type),
    last_update(last_update) {}

Vendor::Vendor(sqlite3_stmt* stmt)
    : mac_prefix(get_column_text(stmt, 1)),
      vendor_name(get_column_text(stmt, 2)),
      is_private(sqlite3_column_int(stmt, 3) != 0),
      block_type(get_column_text(stmt, 4)),
      last_update(get_column_text(stmt, 5)) {}

int Vendor::bind(sqlite3_stmt* stmt) {
    const std::string stripped_prefix = remove_addr_separators(mac_prefix);

    return sqlite3_bind_int64(stmt, 1, prefix_to_id(stripped_prefix)) +
           sqlite3_bind_text(stmt, 2, mac_prefix.c_str(), -1, SQLITE_STATIC) +
           sqlite3_bind_text(stmt, 3, vendor_name.c_str(), -1, SQLITE_STATIC) +
           sqlite3_bind_int(stmt, 4, is_private ? 1 : 0) +
           sqlite3_bind_text(stmt, 5, block_type.c_str(), -1, SQLITE_STATIC) +
           sqlite3_bind_text(stmt, 6, last_update.c_str(), -1, SQLITE_STATIC);
}

std::ostream& operator<<(std::ostream& os, const Vendor& v) {
    os << "MAC prefix   " << v.mac_prefix << "\n"
       << "Vendor name  " << (v.is_private ? "-" : v.vendor_name) << '\n'
       << "Private      " << (v.is_private ? "yes" : "no") << '\n'
       << "Block type   " << (v.is_private ? "-" : v.block_type) << '\n'
       << "Last update  " << (v.is_private ? "-" : v.last_update);
    return os;
}

// Returns a string value stored in column 'coln' of sqlite row. If the value
// is NULL, an empty string is returned.
static inline std::string get_column_text(sqlite3_stmt* stmt, int coln) {
    const unsigned char* text = sqlite3_column_text(stmt, coln);

    if (text) {
        return reinterpret_cast<const char*>(text);
    }
    return "";
}

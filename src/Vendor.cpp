#include <sqlite3.h>
#include <string>

#include "Vendor.hpp"
#include "exception.hpp"
#include "utils.hpp"

static inline std::string get_column_text(sqlite3_stmt* const stmt, const int coln);

Vendor::Vendor(const std::string& line) {
    // In this constructor, size_t values that are marked with '1' typically
    // point to the first element of interest (opening quote, the opening of
    // a CSV field), while values marked with '2' point to its terminating
    // counterpart (closing quote, comma marking the end of the CSV field).
    size_t p1{}, p2{};

    if ((p1 = line.find(',', 0)) == std::string::npos) {
        throw errors::NoCommaError.wrap(line);
    }
    mac_prefix = line.substr(0, p1);
    p1++;

    if (line.at(p1) == '"') {
        // Quoted vendor name (,"Cisco Systems, Inc",)
        size_t eqp1{}, eqp2{};

        if ((eqp1 = line.find("\"\"", p1)) != std::string::npos) {
            // Escaped quotes detection (""Black"" ops) for vendor name
            if ((eqp2 = line.find("\"\"", eqp1 + 1)) == std::string::npos) {
                throw errors::EscapedTermError.wrap(line);
            }

            // Find the vendor name field closure past the second escaped quote
            if ((p2 = line.find('"', eqp2 + 2)) == std::string::npos) {
                throw errors::QuotedTermSeqError.wrap(line);
            }

            // Assign substring to vendor name and replace escaped quotes
            // with single quotes
            vendor_name = line.substr(p1 + 1, p2 - p1 - 1);
            vendor_name.replace(eqp1 - p1 - 1, 2, "\"");
            vendor_name.replace(eqp2 - 2 - p1, 2, "\"");
        } else {
            // Quoted vendor name with no escaped quotes ("Cisco Systems, Inc")
            if ((p2 = line.find("\",", p1 + 1)) == std::string::npos) {
                throw errors::QuotedTermSeqError.wrap(line);
            }
            vendor_name = line.substr(p1 + 1, p2 - p1 - 1);
        }
        p1 = p2 + 2;
    } else if (line.at(p1) == ',') {
        // Private block always has an empty vendor name - skip a comma
        // to reach private designator field.
        vendor_name = "";
        p1++;
    } else {
        // Unquoted vendor name
        if ((p2 = line.find(',', p1 + 1)) == std::string::npos) {
            throw errors::UnquotedTermError.wrap(line);
        }
        vendor_name = line.substr(p1, p2 - p1);
        p1          = p2 + 1;
    }

    if (line.at(p1) == 't') {
        // Private entry contains no more meaningful information
        is_private  = true;
        block_type  = "";
        last_update = "";
        return;
    } else if (line.at(p1) != 'f') {
        // Private designator field must contain either 'true' or 'false'
        throw errors::PrivateInvalidError.wrap(line);
    }

    is_private = false;

    // Skip past 'alse,' to the next field
    p1 += 5;
    if (p1 >= line.length() || line[p1] != ',') {
        throw errors::PrivateTermError.wrap(line);
    }
    p1++;

    // Find the last comma
    if ((p2 = line.find(',', p1)) == std::string::npos) {
        throw errors::BlockTypeTermError.wrap(line);
    }

    block_type  = line.substr(p1, p2 - p1);
    last_update = line.substr(p2 + 1);
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

Vendor::Vendor(sqlite3_stmt* const stmt)
    : mac_prefix(get_column_text(stmt, 1)),
      vendor_name(get_column_text(stmt, 2)),
      is_private(sqlite3_column_int(stmt, 3) != 0),
      block_type(get_column_text(stmt, 4)),
      last_update(get_column_text(stmt, 5)) {}

int Vendor::bind(sqlite3_stmt* const stmt) const {
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
static inline std::string get_column_text(sqlite3_stmt* const stmt, const int coln) {
    const unsigned char* text = sqlite3_column_text(stmt, coln);

    if (text) {
        return reinterpret_cast<const char*>(text);
    }
    return "";
}

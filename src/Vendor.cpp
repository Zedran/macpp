#include <sqlite3.h>
#include <string>

#include "Vendor.hpp"
#include "exception.hpp"
#include "utils.hpp"

static inline void bind(sqlite3_stmt* const stmt, const int coln, const std::string& value);
static inline void bind(sqlite3_stmt* const stmt, const int coln, const int64_t value);
static inline void bind(sqlite3_stmt* const stmt, const int coln, const bool value);

static inline std::string get_column_text(sqlite3_stmt* const stmt, const int coln);

Vendor::Vendor(const std::string& line) {
    constexpr char        COMMA = ',';
    constexpr char        QUOTE = '"';
    constexpr const char* ESCQT = "\"\""; // Escaped double quote
    constexpr const char* QTCOM = "\",";  // Termination of quoted CSV field

    // In this constructor, p1 typically points to the first element of interest
    // (opening quote, the beginning of a CSV field), while p2 points to its
    // terminating counterpart (closing quote, comma at the end of the field).
    size_t p1{}, p2{};

    if ((p1 = line.find(COMMA, 0)) == std::string::npos) {
        throw errors::NoCommaError{line};
    }
    mac_prefix = line.substr(0, p1);
    p1++;

    if (line.at(p1) == QUOTE) {
        // Quoted vendor name (,"Cisco Systems, Inc",)

        // Find the vendor name field closure past the second escaped quote
        if ((p2 = line.find(QTCOM, p1 + 1)) == std::string::npos) {
            throw errors::QuotedTermSeqError{line};
        }

        // Assign substring to vendor name and replace escaped quotes
        // with single quotes
        vendor_name = line.substr(p1 + 1, p2 - p1 - 1);
        if (vendor_name.find(ESCQT) != std::string::npos) {
            replace_escaped_quotes(vendor_name);
        }
        p1 = p2 + 2;
    } else if (line.at(p1) == COMMA) {
        // Private block always has an empty vendor name - skip a comma
        // to reach private designator field.
        vendor_name = "";
        p1++;
    } else {
        // Unquoted vendor name
        if ((p2 = line.find(COMMA, p1 + 1)) == std::string::npos) {
            throw errors::UnquotedTermError{line};
        }
        vendor_name = line.substr(p1, p2 - p1);
        p1          = p2 + 1;
    }

    if (p1 == line.length()) {
        // Private field is empty (comma after vendor name ends the line).
        throw errors::PrivateInvalidError{line};
    }

    if (line.at(p1) == 't') {
        // Private entry contains no more meaningful information
        is_private  = true;
        block_type  = "";
        last_update = "";
        return;
    } else if (line.at(p1) != 'f') {
        // Private designator field must contain either 'true' or 'false'
        throw errors::PrivateInvalidError{line};
    }

    is_private = false;

    // Skip past 'alse,' to the next field
    p1 += 5;
    if (p1 >= line.length() || line[p1] != COMMA) {
        throw errors::PrivateTermError{line};
    }
    p1++;

    // Find the last comma
    if ((p2 = line.find(COMMA, p1)) == std::string::npos) {
        throw errors::BlockTypeTermError{line};
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

void Vendor::bind(sqlite3_stmt* const stmt) const {
    const int64_t id = prefix_to_id(remove_addr_separators(mac_prefix));

    ::bind(stmt, 1, id);
    ::bind(stmt, 2, mac_prefix);
    ::bind(stmt, 3, vendor_name);
    ::bind(stmt, 4, is_private);
    ::bind(stmt, 5, block_type);
    ::bind(stmt, 6, last_update);
}

std::ostream& operator<<(std::ostream& os, const Vendor& v) {
    os << "MAC prefix   " << v.mac_prefix << "\n"
       << "Vendor name  " << (v.is_private ? "-" : v.vendor_name) << '\n'
       << "Private      " << (v.is_private ? "yes" : "no") << '\n'
       << "Block type   " << (v.is_private ? "-" : v.block_type) << '\n'
       << "Last update  " << (v.is_private ? "-" : v.last_update);
    return os;
}

// Binds an int64 to coln of sqlite3 statement.
static inline void bind(sqlite3_stmt* const stmt, const int coln, const int64_t value) {
    if (int code = sqlite3_bind_int64(stmt, coln, value); code != SQLITE_OK) {
        throw errors::BindError.wrap(code);
    }
}

// Binds a boolean value to coln of sqlite3 statement.
static inline void bind(sqlite3_stmt* const stmt, const int coln, const bool value) {
    if (int code = sqlite3_bind_int(stmt, coln, value ? 1 : 0); code != SQLITE_OK) {
        throw errors::BindError.wrap(code);
    }
}

// Binds a string to coln of sqlite3 statement.
static inline void bind(sqlite3_stmt* const stmt, const int coln, const std::string& value) {
    if (int code = sqlite3_bind_text(stmt, coln, value.c_str(), -1, SQLITE_STATIC); code != SQLITE_OK) {
        throw errors::BindError.wrap(code);
    }
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

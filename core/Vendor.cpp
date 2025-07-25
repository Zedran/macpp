#include <sqlite3.h>
#include <string>

#include "Vendor.hpp"
#include "exception.hpp"
#include "utils.hpp"

Vendor::Vendor(const std::string& line) {
    constexpr char        COMMA = ',';
    constexpr char        QUOTE = '"';
    constexpr const char* ESCQT = "\"\""; // Escaped double quote
    constexpr const char* QTCOM = "\",";  // Termination of quoted CSV field

    // In this constructor, p1 typically points to the first element of interest
    // (opening quote, the beginning of a CSV field), while p2 points to its
    // terminating counterpart (closing quote, comma at the end of the field).
    size_t p1, p2;

    if ((p1 = line.find(COMMA, 0)) == std::string::npos) {
        throw errors::NoCommaError{line};
    }
    mac_prefix = prefix_to_int(line.substr(0, p1));
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
        block_type  = Registry::Unknown;
        last_update = "";
        return;
    } else if (line.at(p1) != 'f') {
        // Private designator field must contain either 'true' or 'false'
        throw errors::PrivateInvalidError{line};
    }

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

    block_type  = to_registry(line.substr(p1, p2 - p1));
    last_update = line.substr(p2 + 1);
}

Vendor::Vendor(
    const int64_t      mac_prefix,
    const std::string& vendor_name,
    const Registry     block_type,
    const std::string& last_update
) noexcept : mac_prefix{mac_prefix},
             vendor_name{vendor_name},
             block_type{block_type},
             last_update{last_update} {}

std::ostream& operator<<(std::ostream& os, const Vendor& v) {
    const std::string prefix     = prefix_to_string(v.mac_prefix);
    const bool        is_private = v.vendor_name.empty();

    os << "MAC prefix   " << prefix << '\n'
       << "Vendor name  " << (is_private ? "-" : v.vendor_name) << '\n'
       << "Private      " << (is_private ? "yes" : "no") << '\n'
       << "Block type   " << from_registry(v.block_type) << '\n'
       << "Last update  " << (is_private ? "-" : v.last_update);
    return os;
}

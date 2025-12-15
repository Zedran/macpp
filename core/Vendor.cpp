#include <string>

#include "Vendor.hpp"
#include "exception.hpp"
#include "out.hpp"
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

    if (p1 >= line.length()) {
        throw errors::PrefixTermError{line};
    }

    if (line[p1] == QUOTE) {
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
    } else if (line[p1] == COMMA) {
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

    if (p1 >= line.length()) {
        // Private field is empty (comma after vendor name ends the line).
        throw errors::PrivateInvalidError{line};
    }

    if (line[p1] == 't') {
        // Private entry contains no more meaningful information
        is_private  = true;
        block_type  = Registry::Unknown;
        last_update = "";
        return;
    } else if (line[p1] != 'f') {
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

    block_type  = to_registry(line.substr(p1, p2 - p1));
    last_update = line.substr(p2 + 1);
}

std::ostream& Vendor::write_string_csv(std::ostream& os) const noexcept {
    os << prefix_to_string(mac_prefix) << ',';

    if (has_spec_chars<out::Format::CSV>(vendor_name)) {
        os << '"' << escape_spec_chars<out::Format::CSV>(vendor_name) << "\",";
    } else if (vendor_name.find(',') != std::string::npos) {
        os << '"' << vendor_name << "\",";
    } else {
        os << vendor_name << ',';
    }

    const bool is_private = vendor_name.empty();

    os << std::boolalpha << is_private << ',';

    if (block_type != Registry::Unknown) {
        os << from_registry(block_type);
    }

    return os << ',' << last_update;
}

std::ostream& Vendor::write_string_json(std::ostream& os) const noexcept {
    os << R"({"macPrefix":")" << prefix_to_string(mac_prefix)
       << R"(","vendorName":")";

    if (has_spec_chars<out::Format::JSON>(vendor_name)) {
        os << escape_spec_chars<out::Format::JSON>(vendor_name);
    } else {
        os << vendor_name;
    }

    const bool is_private = vendor_name.empty();

    os << R"(","private":)" << std::boolalpha << is_private << R"(,"blockType":")";

    if (block_type != Registry::Unknown) {
        os << from_registry(block_type);
    }

    return os << R"(","lastUpdate":")" << last_update << R"("})";
}

std::ostream& Vendor::write_string_regular(std::ostream& os) const noexcept {
    const bool is_private = vendor_name.empty();

    return os << "MAC prefix   " << prefix_to_string(mac_prefix) << '\n'
              << "Vendor name  " << (is_private ? "-" : vendor_name) << '\n'
              << "Private      " << (is_private ? "yes" : "no") << '\n'
              << "Block type   " << from_registry(block_type) << '\n'
              << "Last update  " << (is_private ? "-" : last_update);
}

std::ostream& Vendor::write_string_xml(std::ostream& os) const noexcept {
    os << R"(<VendorMapping mac_prefix=")" << prefix_to_string(mac_prefix)
       << R"(" vendor_name=")";

    if (has_spec_chars<out::Format::XML>(vendor_name)) {
        os << escape_spec_chars<out::Format::XML>(vendor_name);
    } else {
        os << vendor_name;
    }

    return os << R"("></VendorMapping>)";
}

std::ostream& operator<<(std::ostream& os, const Vendor& v) {
    switch (out::get_format(os)) {
    case out::Format::CSV:  return v.write_string_csv(os);
    case out::Format::JSON: return v.write_string_json(os);
    case out::Format::XML:  return v.write_string_xml(os);
    default:                return v.write_string_regular(os);
    }
}

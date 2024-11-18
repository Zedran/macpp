#include <regex>
#include <sstream>
#include <string>

#include "Vendor.hpp"

Vendor::Vendor(const std::string line) {
    // :0C,"Cisco Systems, Inc.",f
    const std::regex quotes{R"(:[0123456789ABCDEF]{2},".*",[ft])"};

    std::string        priv_str;
    std::smatch        submatch;
    std::istringstream line_stream(line);

    std::getline(line_stream, mac_prefix, ',');

    if (std::regex_search(line, submatch, quotes)) {
        vendor_name = submatch.str();
        line_stream.seekg(vendor_name.length(), std::ios::cur);
        vendor_name.erase(0, 5);
        vendor_name.erase(vendor_name.length() - 3, 3);
    } else {
        std::getline(line_stream, vendor_name, ',');
    }

    std::getline(line_stream, priv_str, ',');
    std::getline(line_stream, block_type, ',');
    std::getline(line_stream, last_update);

    is_private = (priv_str == "true") ? true : false;
}

Vendor::Vendor(std::string mac_prefix, std::string vendor_name, bool is_private, std::string block_type, std::string last_update)
    : mac_prefix(mac_prefix),
      vendor_name(vendor_name),
      is_private(is_private),
      block_type(block_type),
      last_update(last_update) {}

std::ostream& operator<<(std::ostream& os, const Vendor& v) {
    os << "MAC prefix   " << v.mac_prefix << "\n"
       << "Vendor name  " << (v.is_private ? "-" : v.vendor_name) << '\n'
       << "Private      " << (v.is_private ? "yes" : "no") << '\n'
       << "Block type   " << (v.is_private ? "-" : v.block_type) << '\n'
       << "Last update  " << (v.is_private ? "-" : v.last_update);
    return os;
}

#pragma once

#include <iostream>
#include <string>

struct Vendor {
    std::string mac_prefix;
    std::string vendor_name;
    bool        is_private;
    std::string block_type;
    std::string last_update;

    Vendor();

    // Creates a Vendor struct from a comma-separated CSV line.
    Vendor(std::string line);

    Vendor(std::string mac_prefix, std::string vendor_name, bool is_private, std::string block_type, std::string last_update);

    friend std::ostream& operator<<(std::ostream& os, const Vendor& v);
};

#pragma once

#include <iostream>
#include <string>

#include "Registry.hpp"

struct Vendor {
    int64_t     mac_prefix;
    std::string vendor_name;
    Registry    block_type;
    std::string last_update;

    // Creates a Vendor struct from a comma-separated CSV line.
    Vendor(const std::string& line);

    Vendor(
        const int64_t      mac_prefix,
        const std::string& vendor_name,
        const Registry     block_type,
        const std::string& last_update
    ) noexcept;

    friend std::ostream& operator<<(std::ostream& os, const Vendor& v);
};

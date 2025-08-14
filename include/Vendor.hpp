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

    // Formats Vendor data into CSV line and writes it to os.
    std::ostream& write_string_csv(std::ostream& os) const noexcept;

    // Writes Vendor data in the default format to os.
    std::ostream& write_string_regular(std::ostream& os) const noexcept;

    // Formats Vendor data into JSON dictionary and writes it to os.
    std::ostream& write_string_json(std::ostream& os) const noexcept;

    friend std::ostream& operator<<(std::ostream& os, const Vendor& v);
};

#pragma once

#include <iostream>
#include <string>

#include "Registry.hpp"

struct Vendor {
    int64_t     mac_prefix;
    std::string vendor_name;
    bool        is_private;
    Registry    block_type;
    std::string last_update;

    // Creates a Vendor struct from a comma-separated CSV line.
    Vendor(const std::string& line);

    constexpr Vendor(
        const int64_t  mac_prefix,
        std::string    vendor_name,
        const bool     is_private,
        const Registry block_type,
        std::string    last_update
    ) noexcept : mac_prefix{mac_prefix},
                 vendor_name{std::move(vendor_name)},
                 is_private{is_private},
                 block_type{block_type},
                 last_update{std::move(last_update)} {}

    // Formats Vendor data into CSV line and writes it to os.
    std::ostream& write_string_csv(std::ostream& os) const noexcept;

    // Writes Vendor data in the default format to os.
    std::ostream& write_string_regular(std::ostream& os) const noexcept;

    // Formats Vendor data into JSON dictionary and writes it to os.
    std::ostream& write_string_json(std::ostream& os) const noexcept;

    // Formats Vendor data into XML VendorMapping and writes it to os.
    std::ostream& write_string_xml(std::ostream& os) const noexcept;

    bool operator==(const Vendor& other) const = default;

    friend std::ostream& operator<<(std::ostream& os, const Vendor& v);
};

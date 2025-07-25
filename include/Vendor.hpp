#pragma once

#include <iostream>
#include <string>

#include "cache/Stmt.hpp"

struct Vendor {
    int64_t     mac_prefix;
    std::string vendor_name;
    Registry    block_type;
    std::string last_update;

    // Creates a Vendor struct from a comma-separated CSV line.
    Vendor(const std::string& line);

    Vendor(
        const std::string& mac_prefix,
        const std::string& vendor_name,
        const std::string& block_type,
        const std::string& last_update
    );

    // Reconstructs Vendor from SQLite row.
    Vendor(const Stmt& stmt);

    // Binds struct members to the insert statement.
    void bind(const Stmt& stmt) const;

    friend std::ostream& operator<<(std::ostream& os, const Vendor& v);
};

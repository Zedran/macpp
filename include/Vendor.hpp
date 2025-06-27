#pragma once

#include <iostream>
#include <sqlite3.h>
#include <string>

#include "SQLite.hpp"

struct Vendor {
    std::string mac_prefix;
    std::string vendor_name;
    bool        is_private;
    std::string block_type;
    std::string last_update;

    Vendor();

    // Creates a Vendor struct from a comma-separated CSV line.
    Vendor(const std::string& line);

    Vendor(
        const std::string& mac_prefix,
        const std::string& vendor_name,
        const bool         is_private,
        const std::string& block_type,
        const std::string& last_update
    );

    // Reconstructs Vendor from SQLite row.
    Vendor(const Stmt& stmt);

    // Binds struct members to the insert statement.
    void bind(Stmt& stmt) const;

    friend std::ostream& operator<<(std::ostream& os, const Vendor& v);
};

#pragma once

#include <iostream>
#include <sqlite3.h>
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

    // Reconstructs Vendor from SQLite row.
    Vendor(sqlite3_stmt* stmt);

    // Binds struct members to the insert statement. Returns an aggregation
    // of the SQLite3 status codes. If the return value is higher than 0,
    // at least one of the bind calls failed.
    int bind(sqlite3_stmt* stmt);

    friend std::ostream& operator<<(std::ostream& os, const Vendor& v);
};

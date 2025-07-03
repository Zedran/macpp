#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "Conn.hpp"
#include "Vendor.hpp"

// Wrapper for read-only database connection.
class ConnR : public Conn {
    // Signals whether check() member function has been called.
    static std::once_flag db_checked;

    // Signals whether static once_flag class members should be respected.
    bool override_once_flags;

    // Checks whether the database file exists, has a proper structure
    // and the data is present.
    void check();

    // Helper function for check(). Returns a number of records present
    // in the vendors table.
    int64_t count_records();

public:
    ConnR() noexcept;

    // Constructs new read-only database connection given the database path.
    // If override_once_flags is set to true, the constructor ignores static
    // once_flag class members that prevent doing extra work during
    // instantialization in multithreaded context. This boolean switch should
    // only be set to true for testing purposes.
    ConnR(const std::string& path, const bool override_once_flags = false);

    ConnR(const ConnR&)            = delete;
    ConnR& operator=(const ConnR&) = delete;

    ~ConnR() = default;

    // Searches for records with given MAC address.
    std::vector<Vendor> find_by_addr(const std::string& addr) const;

    // Searches for records with given vendor name name.
    std::vector<Vendor> find_by_name(const std::string& name) const;
};

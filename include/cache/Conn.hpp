#pragma once

#include <mutex>
#include <sqlite3.h>
#include <string>

#include "config.hpp"

// Base class wrapping SQLite3 database connection.
class Conn {
    // Default busy timeout value for the connection.
    static constexpr int BUSY_TIMEOUT_MS = 5000;

    // Signals whether sqlite3_initialize() function has been called.
    static std::once_flag sqlite_initialized;

protected:
    // Pointer to the sqlite3 object.
    sqlite3* conn;

    // Result code returned by sqlite3_open_v2 function.
    int sqlite_open_rc;

    // Constructs new Conn, given a path to the database file and open mode.
    // Initializes SQLite on first instantiation.
    Conn(const std::string& path, const int flags);

    // Returns true if the vendors table exists in the database.
    bool has_table() const;

public:
    // Current SQLite user_version value. Incremented manually on every cache
    // modification.
    static constexpr int EXPECTED_CACHE_VERSION = __MACPP_CACHE_VERSION__;

    Conn(const Conn&)            = delete;
    Conn& operator=(const Conn&) = delete;

    // Closes connection with the database.
    virtual ~Conn();

    // Returns a pointer to sqlite3 object.
    sqlite3* get() const noexcept;

    // Returns the result code of the sqlite3_open_v2 function.
    int rc() const noexcept;

    // Returns user_version value for the database. Not to be confused
    // with Conn::EXPECTED_CACHE_VERSION, which indicates cache version used by
    // the application.
    int version() const;
};

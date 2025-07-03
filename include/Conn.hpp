#pragma once

#include <sqlite3.h>
#include <string>

// Base class wrapping SQLite3 database connection.
class Conn {
    // Default busy timeout value for the connection.
    static constexpr int BUSY_TIMEOUT_MS = 5000;

protected:
    // Pointer to the sqlite3 object.
    sqlite3* conn;

    // Result code returned by sqlite3_open_v2 function.
    int sqlite_open_rc;

    Conn() noexcept;

    // Constructs new Conn, given a path to the database file and open mode.
    Conn(const std::string& path, const int flags) noexcept;

    // Returns true if the vendors table exists in the database.
    bool has_table();

    // Returns the result code of the sqlite3_open_v2 function.
    int rc();

public:
    Conn(const Conn&)            = delete;
    Conn& operator=(const Conn&) = delete;

    // Closes connection with the database.
    virtual ~Conn();

    // Returns a pointer to sqlite3 object.
    sqlite3* get() const noexcept;
};

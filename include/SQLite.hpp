#pragma once

#include <sqlite3.h>
#include <string>

// A RAII wrapper for sqlite3_stmt object.
class Stmt {
    // Result code returned by sqlite3_prepare function.
    int prepare_rc;

    // Pointer to SQLite statement object.
    sqlite3_stmt* stmt;

public:
    Stmt(sqlite3* const conn, const char* str_stmt);

    Stmt(sqlite3* const conn, const std::string& str_stmt);

    ~Stmt();

    // Returns pointer to the wrapped sqlite3_stmt object.
    sqlite3_stmt* get() const noexcept;

    // Returns true if statement has been prepared correctly.
    bool ok() const noexcept;

    // Returns result code returned by sqlite3_prepare function.
    int rc() const noexcept;

    // Resets the statement.
    int reset() const noexcept;

    // Steps the statement.
    int step() const noexcept;
};

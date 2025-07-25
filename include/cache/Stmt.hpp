#pragma once

#include <cstdint>
#include <sqlite3.h>
#include <string>

#include "Registry.hpp"

// A RAII wrapper for sqlite3_stmt object.
class Stmt {
    // Result code returned by sqlite3_prepare function.
    int prepare_rc;

    // Pointer to SQLite statement object.
    sqlite3_stmt* stmt;

public:
    Stmt(sqlite3* const conn, const char* str_stmt) noexcept;

    Stmt(sqlite3* const conn, const std::string& str_stmt) noexcept;

    Stmt(const Stmt&)            = delete;
    Stmt& operator=(const Stmt&) = delete;

    ~Stmt();

    // Binds an int64 to coln of the statement. Returns SQLite result code.
    int bind(const int coln, const int64_t value) const noexcept;

    // Binds a Registry enum class value to coln of the statement.
    // Returns SQLite result code.
    int bind(const int coln, const Registry value) const noexcept;

    // Binds a string to coln of the statement. Returns SQLite result code.
    int bind(const int coln, const std::string& value) const noexcept;

    // Returns pointer to the wrapped sqlite3_stmt object.
    sqlite3_stmt* get() const noexcept;

    // Generic function for extracting value from SQLite table column.
    // Supported types:
    //   - std::string
    //   - bool
    //   - int32
    //   - int64
    //   - Registry enum class
    //
    // For strings, if column value is NULL, an empty string is returned.
    // This should never happen unless the cache has been tampered with.
    template <typename T>
        requires(
            std::same_as<T, std::string> ||
            std::same_as<T, Registry> ||
            std::same_as<T, int32_t> ||
            std::same_as<T, int64_t>
        )
    T get_col(const int coln) const noexcept {
        if constexpr (std::is_same_v<T, std::string>) {
            const unsigned char* text = sqlite3_column_text(stmt, coln);

            if (text) [[likely]] {
                return std::string{reinterpret_cast<const char*>(text)};
            } else {
                return "";
            }
        }

        if constexpr (std::is_same_v<T, Registry>) {
            return static_cast<Registry>(sqlite3_column_int(stmt, coln));
        }

        if constexpr (std::is_same_v<T, int32_t>) {
            return sqlite3_column_int(stmt, coln);
        }

        if constexpr (std::is_same_v<T, int64_t>) {
            return sqlite3_column_int64(stmt, coln);
        }
    }

    // Returns true if statement has been prepared correctly.
    bool good() const noexcept;

    // Returns result code returned by sqlite3_prepare function.
    int rc() const noexcept;

    // Resets the statement.
    int reset() const noexcept;

    // Steps the statement.
    int step() const noexcept;

    // Returns true if statement has been prepared correctly.
    explicit operator bool() const noexcept;
};

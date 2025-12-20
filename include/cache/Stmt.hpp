#pragma once

#include <cstdint>
#include <sqlite3.h>
#include <string>

#include "Registry.hpp"
#include "Vendor.hpp"

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
    int bind(const int coln, const int64_t value) noexcept;

    // Binds a Registry enum class value to coln of the statement.
    // Registry::Unknown is bound as NULL.
    // Returns SQLite result code.
    int bind(const int coln, const Registry value) noexcept;

    // Binds a string to coln of the statement. Returns SQLite result code.
    // Empty string is bound as NULL.
    int bind(const int coln, const std::string& value) noexcept;

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
    // For strings, if a column value is NULL, an empty string is returned.
    // For Registry values, encountering NULL or a value outside of the defined
    // enum class range causes Registry::Unknown to be returned.
    template <typename T>
        requires(
            std::same_as<T, std::string> ||
            std::same_as<T, Registry> ||
            std::same_as<T, bool> ||
            std::same_as<T, int32_t> ||
            std::same_as<T, int64_t>
        )
    T get_col(const int coln) const noexcept {
        if constexpr (std::is_same_v<T, std::string>) {
            if (sqlite3_column_type(stmt, coln) == SQLITE_NULL) {
                return "";
            }
            const unsigned char* text = sqlite3_column_text(stmt, coln);
            return std::string{reinterpret_cast<const char*>(text)};
        }

        if constexpr (std::is_same_v<T, Registry>) {
            if (sqlite3_column_type(stmt, coln) == SQLITE_NULL) {
                return Registry::Unknown;
            }

            int value = sqlite3_column_int(stmt, coln);
            if (value < static_cast<int>(Registry::Unknown) || value > static_cast<int>(Registry::MA_S)) {
                return Registry::Unknown;
            }
            return static_cast<Registry>(value);
        }

        if constexpr (std::is_same_v<T, bool>) {
            return sqlite3_column_int(stmt, coln) == 1 ? true : false;
        }

        if constexpr (std::is_same_v<T, int32_t>) {
            return sqlite3_column_int(stmt, coln);
        }

        if constexpr (std::is_same_v<T, int64_t>) {
            return sqlite3_column_int64(stmt, coln);
        }
    }

    // Retrieves Vendor instance from SQLite row.
    Vendor get_row() noexcept;

    // Returns true if statement has been prepared correctly.
    bool good() const noexcept;

    // Binds Vendor instance to the statement.
    void insert_row(const Vendor& v);

    // Returns result code returned by sqlite3_prepare function.
    int rc() const noexcept;

    // Resets the statement.
    int reset() noexcept;

    // Steps the statement.
    int step() noexcept;

    // Returns true if statement has been prepared correctly.
    explicit operator bool() const noexcept;
};

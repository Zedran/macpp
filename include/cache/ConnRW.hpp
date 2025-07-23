#pragma once

#include <mutex>
#include <string>

#include "Conn.hpp"
#include "Vendor.hpp"

// Wrapper for read-write database connection.
class ConnRW : public Conn {
    // More than twice the length of the longest CSV line.
    static constexpr size_t MAX_LINE_LENGTH = 300;

    static constexpr const char* INSERT_STMT =
        "INSERT INTO vendors "
        "(prefix, name, block, updated) "
        "VALUES (?1, ?2, ?3, ?4)";

    // Signals whether table records were dropped before insertions.
    static std::once_flag cleared_before_insert;

    // Signals whether prepare_database has been called.
    static std::once_flag db_prepared;

    // Signals whether static once_flag class members should be respected.
    bool override_once_flags;

    // Signals whether database transaction is opened.
    bool transaction_open;

    // Creates table vendors in the database. Returns the result code
    // by sqlite3_exec.
    int create_table() const noexcept;

    // Drops table vendors. Returns the result code reported by sqlite3_exec.
    int drop_table() const noexcept;

    // Handles the initial preparation stage. Ensures the database exists
    // and has been correctly formatted.
    void prepare_db() const;

public:
    ConnRW() noexcept;

    // Constructs new read-write database connection given the database path.
    // If the file is not present, it is created.
    // If override_once_flags is set to true, the constructor ignores static
    // once_flag class members that prevent doing extra work during
    // instantiation in multithreaded context. This boolean switch should
    // only be set to true for testing purposes.
    ConnRW(const std::string& path, const bool override_once_flags = false);

    ConnRW(const ConnRW&)            = delete;
    ConnRW& operator=(const ConnRW&) = delete;

    // Rolls back currently running transaction.
    ~ConnRW();

    // Opens database transaction.
    int begin() noexcept;

    // Deletes all records from the vendors table.
    int clear_table() const noexcept;

    // Commits database transaction.
    int commit() noexcept;

    // Opens a new transaction, deletes all records from the vendors table
    // (call_once), then parses CSV lines contained in is and inserts them
    // into the database. The function expects the first line to be
    // the header line - it is discarded. If no exception is thrown,
    // the transaction is committed. If the exceptional path does not
    // result in ConnRW's destruction, it is the caller's responsibility
    // to rollback the transaction.
    void insert(std::istream& is);

    // Inserts a single Vendors into the database. The caller is responsible
    // for managing transactions.
    void insert(const Vendor& v) const;

    // Reverts uncommitted database transaction.
    int rollback() noexcept;

    // Assigns the user_version value as specifed. Returns the result code
    // reported by sqlite3_exec.
    int set_version(const int version) const noexcept;
};

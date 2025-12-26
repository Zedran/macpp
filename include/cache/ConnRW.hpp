#pragma once

#include <iostream>
#include <mutex>
#include <source_location>
#include <string>

#include "Conn.hpp"

// Wrapper for read-write database connection.
class ConnRW : public Conn {
    // More than twice the length of the longest CSV line.
    static constexpr size_t MAX_LINE_LENGTH = 300;

    static constexpr const char* CREATE_TABLE_STMT =
        "CREATE TABLE vendors ("
        "prefix  INTEGER PRIMARY KEY,"
        "name    TEXT,"
        "private BOOLEAN NOT NULL,"
        "block   INTEGER,"
        "updated TEXT"
        ")";

    static constexpr const char* INSERT_STMT =
        "INSERT INTO vendors "
        "(prefix, name, private, block, updated) "
        "VALUES (?1, ?2, ?3, ?4, ?5)";

    // Signals whether prepare_database has been called.
    static std::once_flag db_prepared;

    // Signals whether static once_flag class members should be respected.
    bool override_once_flags;

    // Signals whether database transaction is opened.
    bool transaction_open;

    // Creates table vendors in the database. Throws CacheError if a SQLite
    // error is encountered.
    void create_table();

    // Performs database modifications on update. Inserts custom entries
    // and modifies a few existing ones. Data is hard-coded for simplicity.
    // Parameter err is used to redirect warnings for testing.
    // Throws if a SQLite error is encountered.
    void customize_db(std::ostream& err);

    // Drops table vendors. Throws CacheError if a SQLite error is encountered.
    void drop_table();

    // Handles the initial preparation stage. Ensures the database exists
    // and has been correctly formatted.
    void prepare_db();

public:
    ConnRW() noexcept;

    // Constructs new read-write database connection given the database path.
    // If the file is not present, it is created.
    // If override_once_flags is set to true, the constructor ignores static
    // once_flag db_prepared that prevents doing extra work during
    // instantiation. This boolean switch should only be set to true
    // for testing purposes.
    ConnRW(const std::string& path, const bool override_once_flags = false);

    ConnRW(const ConnRW&)            = delete;
    ConnRW& operator=(const ConnRW&) = delete;

    // Rolls back currently running transaction.
    ~ConnRW();

    // Opens database transaction. Throws CacheError if a SQLite error
    // is encountered.
    void begin();

    // Deletes all records from the vendors table. Throws CacheError
    // if a SQLite error is encountered.
    void clear_table();

    // Commits database transaction. Throws CacheError if a SQLite error
    // is encountered.
    void commit();

    // Constructs a statement from string literal and executes it.
    // Throws CacheError if rc is not SQLITE_OK or SQLITE_DONE.
    void exec(std::string_view stmt_str, std::source_location loc = std::source_location::current());

    // Opens a new transaction, parses CSV lines contained in is and
    // inserts them into the database. The function expects the first line
    // to be the header line - it is discarded. If no exception is thrown,
    // the transaction is committed.
    // If update is true, the function deletes all records from the vendors
    // table before inserting new ones and calls the customize_db member
    // function after all the records from is are transfered to the database.
    // Optional parameter err is used to redirect warnings for testing.
    void insert(std::istream& is, const bool update, std::ostream& err = std::cerr);

    // Reverts uncommitted database transaction. Returns SQLite result code.
    int rollback() noexcept;

    // Assigns the user_version value as specifed. Commits database transaction.
    // Throws CacheError if a SQLite error is encountered.
    void set_version(const int version);
};

#include <cassert>

#include "ConnRW.hpp"
#include "Stmt.hpp"
#include "Vendor.hpp"
#include "exception.hpp"

std::once_flag ConnRW::db_checked{};
std::once_flag ConnRW::table_created{};
std::once_flag ConnRW::cleared_before_insert{};

ConnRW::ConnRW() noexcept : Conn{} {}

ConnRW::ConnRW(const std::string& path, const bool override_once_flags)
    : Conn(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE),
      override_once_flags{override_once_flags},
      transaction_open{false} {
    if (sqlite_open_rc != SQLITE_OK) {
        throw errors::CacheError{"open", __func__, sqlite_open_rc};
    }

    if (!override_once_flags) [[likely]] {
        std::call_once(db_checked, [&] { check(); });

        std::call_once(table_created, [&] {
            if (!has_table()) {
                if (int rc = create_table(); rc != SQLITE_OK) {
                    throw errors::CacheError{"create_table", __func__, rc};
                }
            }
        });
    } else [[unlikely]] {
        check();
        if (!has_table()) {
            if (int rc = create_table(); rc != SQLITE_OK) {
                throw errors::CacheError{"create_table", __func__, rc};
            }
        }
    }
}

ConnRW::~ConnRW() {
    if (transaction_open) {
        [[maybe_unused]] const int rc = rollback();
        assert(rc == SQLITE_OK);
    }
}

int ConnRW::begin() noexcept {
    int rc = sqlite3_exec(conn, "BEGIN;", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK) {
        transaction_open = true;
    }
    return rc;
}

void ConnRW::check() const {
    const Stmt stmt{conn, "PRAGMA schema_version"};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, stmt.rc()};
    }

    if (int rc = stmt.step(); rc == SQLITE_NOTADB) {
        // File is not a database and is not empty
        throw errors::CacheError{"not a cache file", __func__, rc};
    }
}

int ConnRW::clear_table() const noexcept {
    return sqlite3_exec(conn, "DELETE FROM vendors;", nullptr, nullptr, nullptr);
}

int ConnRW::commit() noexcept {
    int rc = sqlite3_exec(conn, "COMMIT;", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK) {
        transaction_open = false;
    }
    return rc;
}

int ConnRW::create_table() const noexcept {
    static constexpr const char* create_table_stmt =
        "CREATE TABLE vendors ("
        "id      INTEGER PRIMARY KEY,"
        "addr    TEXT NOT NULL,"
        "name    TEXT NOT NULL,"
        "private BOOLEAN,"
        "block   TEXT NOT NULL,"
        "updated TEXT NOT NULL"
        ")";

    return sqlite3_exec(conn, create_table_stmt, nullptr, nullptr, nullptr);
}

void ConnRW::insert(std::istream& is) {
    begin();

    if (!override_once_flags) [[likely]] {
        std::call_once(cleared_before_insert, [&] { clear_table(); });
    }

    const Stmt stmt{conn, INSERT_STMT};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, conn};
    }

    std::string line;

    int rc;

    // Discard the header line
    std::getline(is, line);

    while (std::getline(is, line)) {
        if (line.empty() || line.length() > MAX_LINE_LENGTH) {
            continue;
        }
        Vendor v{line};
        v.bind(stmt);

        if (rc = stmt.step(); rc != SQLITE_DONE) {
            throw errors::CacheError{"step", __func__, rc};
        }

        if (rc = stmt.reset(); rc != SQLITE_OK) {
            throw errors::CacheError{"reset", __func__, rc};
        }
    }

    commit();
}

void ConnRW::insert(const Vendor& v) const {
    const Stmt stmt{conn, INSERT_STMT};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, conn};
    }

    v.bind(stmt);

    int rc;

    if (rc = stmt.step(); rc != SQLITE_DONE) {
        throw errors::CacheError{"step", __func__, rc};
    }

    if (rc = stmt.reset(); rc != SQLITE_OK) {
        throw errors::CacheError{"reset", __func__, rc};
    }
}

int ConnRW::rollback() noexcept {
    int rc = sqlite3_exec(conn, "ROLLBACK;", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK) {
        transaction_open = false;
    }
    return rc;
}

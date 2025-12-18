#include <cassert>

#include "Vendor.hpp"
#include "cache/ConnRW.hpp"
#include "cache/Stmt.hpp"
#include "exception.hpp"

std::once_flag ConnRW::db_prepared{};

ConnRW::ConnRW() noexcept : Conn{} {}

ConnRW::ConnRW(const std::string& path, const bool override_once_flags)
    : Conn(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE),
      override_once_flags{override_once_flags},
      transaction_open{false} {
    if (sqlite_open_rc != SQLITE_OK) {
        throw errors::CacheError{"open", __func__, sqlite_open_rc};
    }

    if (!override_once_flags) [[likely]] {
        std::call_once(db_prepared, [&] { prepare_db(); });
    } else [[unlikely]] {
        prepare_db();
    }
}

ConnRW::~ConnRW() {
    if (transaction_open) {
        [[maybe_unused]] const int rc = rollback();
        assert(rc == SQLITE_OK);
    }
}

int ConnRW::begin() noexcept {
    assert(!transaction_open && "transaction already open");

    int rc = sqlite3_exec(conn, "BEGIN", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK) {
        transaction_open = true;
    }
    return rc;
}

int ConnRW::clear_table() const noexcept {
    return sqlite3_exec(conn, "DELETE FROM vendors", nullptr, nullptr, nullptr);
}

int ConnRW::commit() noexcept {
    int rc = sqlite3_exec(conn, "COMMIT", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK) {
        transaction_open = false;
    }
    return rc;
}

int ConnRW::create_table() const noexcept {
    return sqlite3_exec(conn, CREATE_TABLE_STMT, nullptr, nullptr, nullptr);
}

int ConnRW::drop_table() const noexcept {
    return sqlite3_exec(conn, "DROP TABLE IF EXISTS vendors", nullptr, nullptr, nullptr);
}

void ConnRW::insert(std::istream& is, const bool with_clear) {
    if (int rc = begin(); rc != SQLITE_OK) {
        throw errors::CacheError{"begin", __func__, rc};
    }

    if (with_clear) {
        clear_table();
    }

    const Stmt stmt{conn, INSERT_STMT};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, stmt.rc()};
    }

    std::string line;

    // Discard the header line
    std::getline(is, line);

    while (std::getline(is, line)) {
        if (!line.empty() && line.length() <= MAX_LINE_LENGTH) {
            stmt.insert_row(Vendor{line});
        }
    }

    if (int rc = commit(); rc != SQLITE_OK) {
        throw errors::CacheError{"commit", __func__, rc};
    }
}

void ConnRW::prepare_db() const {
    bool needs_table;

    if (version() != EXPECTED_CACHE_VERSION) {
        if (int rc = drop_table(); rc != SQLITE_OK) {
            throw errors::CacheError{"drop", __func__, rc};
        }
        needs_table = true;
        set_version(EXPECTED_CACHE_VERSION);
    } else {
        needs_table = !has_table();
    }

    if (needs_table) {
        if (int rc = create_table(); rc != SQLITE_OK) {
            throw errors::CacheError{"create_table", __func__, rc};
        }
    }
}

int ConnRW::rollback() noexcept {
    assert(transaction_open && "transaction has already been comitted");

    int rc = sqlite3_exec(conn, "ROLLBACK", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK) {
        transaction_open = false;
    }
    return rc;
}

int ConnRW::set_version(const int version) const noexcept {
    return sqlite3_exec(
        conn,
        ("PRAGMA user_version = " + std::to_string(version)).c_str(),
        nullptr,
        nullptr,
        nullptr
    );
}

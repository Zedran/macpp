#include <cassert>

#include "cache/Conn.hpp"
#include "cache/Stmt.hpp"
#include "exception.hpp"

std::once_flag Conn::sqlite_initialized{};

Conn::Conn() noexcept : conn{nullptr}, sqlite_open_rc{SQLITE_NOTADB} {}

Conn::Conn(const std::string& path, const int flags) : conn{nullptr} {
    std::call_once(sqlite_initialized, [&] {
        if (sqlite_open_rc = sqlite3_initialize(); sqlite_open_rc != SQLITE_OK) {
            throw errors::CacheError{"sqlite3_initialize failed", __func__, sqlite_open_rc};
        }
    });

    if (sqlite_open_rc = sqlite3_open_v2(path.c_str(), &conn, flags | SQLITE_OPEN_URI, nullptr); sqlite_open_rc != SQLITE_OK) {
        return;
    }
    sqlite_open_rc = sqlite3_busy_timeout(conn, BUSY_TIMEOUT_MS);
}

Conn::~Conn() {
    [[maybe_unused]] const int rc = sqlite3_close(conn);
    assert(rc == SQLITE_OK);
}

sqlite3* Conn::get() const noexcept {
    return conn;
}

bool Conn::has_table() const {
    const Stmt stmt{conn, "SELECT 1 FROM sqlite_master WHERE type='table' AND name='vendors'"};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, stmt.rc()};
    }

    const int rc = stmt.step();

    if (rc == SQLITE_ROW) {
        return true;
    } else if (rc == SQLITE_DONE) {
        return false;
    } else {
        throw errors::CacheError{"step", __func__, rc};
    }
}

int Conn::rc() const noexcept {
    return sqlite_open_rc;
}

int Conn::version() const {
    const Stmt stmt{conn, "PRAGMA user_version"};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, stmt.rc()};
    }

    if (int rc = stmt.step(); rc == SQLITE_NOTADB) {
        // File is not a database and is not empty
        throw errors::CacheError{"not a cache file", __func__, rc};
    } else if (rc != SQLITE_ROW) {
        throw errors::CacheError{"version retrieval", __func__, rc};
    }

    return stmt.get_col<int>(0);
}

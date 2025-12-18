#include <cassert>

#include "Registry.hpp"
#include "cache/Stmt.hpp"
#include "exception.hpp"

Stmt::Stmt(sqlite3* const conn, const char* str_stmt) noexcept
    : prepare_rc{sqlite3_prepare_v2(conn, str_stmt, -1, &stmt, nullptr)} {}

Stmt::Stmt(sqlite3* const conn, const std::string& str_stmt) noexcept
    : prepare_rc{sqlite3_prepare_v2(conn, str_stmt.c_str(), -1, &stmt, nullptr)} {}

Stmt::~Stmt() {
    [[maybe_unused]] const int rc = sqlite3_finalize(stmt);
    assert(rc == SQLITE_OK || rc == SQLITE_NOTADB);
}

int Stmt::bind(const int coln, const int64_t value) noexcept {
    return sqlite3_bind_int64(stmt, coln, value);
}

int Stmt::bind(const int coln, const std::string& value) noexcept {
    return sqlite3_bind_text(stmt, coln, value.c_str(), -1, SQLITE_STATIC);
}

int Stmt::bind(const int coln, const Registry value) noexcept {
    return sqlite3_bind_int(stmt, coln, static_cast<int>(value));
}

sqlite3_stmt* Stmt::get() const noexcept {
    return stmt;
}

Vendor Stmt::get_row() noexcept {
    return Vendor{
        get_col<int64_t>(0),
        get_col<std::string>(1),
        get_col<Registry>(2),
        get_col<std::string>(3),
    };
}

bool Stmt::good() const noexcept {
    return prepare_rc == SQLITE_OK;
}

void Stmt::insert_row(const Vendor& v) {
    int rc;

    if (rc = bind(1, v.mac_prefix); rc != SQLITE_OK) {
        throw errors::CacheError{"col1", __func__, rc};
    }
    if (rc = bind(2, v.vendor_name); rc != SQLITE_OK) {
        throw errors::CacheError{"col2", __func__, rc};
    }
    if (rc = bind(3, v.block_type); rc != SQLITE_OK) {
        throw errors::CacheError{"col3", __func__, rc};
    }
    if (rc = bind(4, v.last_update); rc != SQLITE_OK) {
        throw errors::CacheError{"col4", __func__, rc};
    }

    if (rc = step(); rc != SQLITE_DONE) {
        throw errors::CacheError{"step", __func__, rc};
    }
    if (rc = reset(); rc != SQLITE_OK) {
        throw errors::CacheError{"reset", __func__, rc};
    }
}

int Stmt::rc() const noexcept {
    return prepare_rc;
}

int Stmt::reset() noexcept {
    return sqlite3_reset(stmt);
}

int Stmt::step() noexcept {
    return sqlite3_step(stmt);
}

Stmt::operator bool() const noexcept {
    return good();
}

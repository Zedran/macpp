#include <cassert>
#include <source_location>

#include "Registry.hpp"
#include "cache/Stmt.hpp"
#include "exception.hpp"
#include "utils.hpp"

Stmt::Stmt(sqlite3* const conn, const char* str_stmt, const std::source_location loc) {
    if (const int rc = sqlite3_prepare_v2(conn, str_stmt, -1, &stmt, nullptr); rc != SQLITE_OK) {
        throw errors::CacheError{"prepare", fmt_loc(loc), rc};
    }
}

Stmt::Stmt(sqlite3* const conn, const std::string& str_stmt, const std::source_location loc) {
    if (const int rc = sqlite3_prepare_v2(conn, str_stmt.c_str(), -1, &stmt, nullptr); rc != SQLITE_OK) {
        throw errors::CacheError{"prepare", fmt_loc(loc), rc};
    }
}

Stmt::~Stmt() {
    [[maybe_unused]] const int rc = sqlite3_finalize(stmt);
    assert(rc == SQLITE_OK || rc == SQLITE_NOTADB);
}

void Stmt::bind(const int coln, const int64_t value, const std::source_location loc) {
    if (const int rc = sqlite3_bind_int64(stmt, coln, value); rc != SQLITE_OK) {
        throw errors::CacheError{"col" + std::to_string(coln), fmt_loc(loc), rc};
    }
}

void Stmt::bind(const int coln, const Registry value, const std::source_location loc) {
    int rc;

    if (value == Registry::Unknown) {
        rc = sqlite3_bind_null(stmt, coln);
    } else {
        rc = sqlite3_bind_int(stmt, coln, static_cast<int>(value));
    }

    if (rc != SQLITE_OK) {
        throw errors::CacheError{"col" + std::to_string(coln), fmt_loc(loc), rc};
    }
}

void Stmt::bind(const int coln, const std::string& value, const std::source_location loc) {
    int rc;

    if (value.empty()) {
        rc = sqlite3_bind_null(stmt, coln);
    } else {
        rc = sqlite3_bind_text(stmt, coln, value.c_str(), -1, SQLITE_STATIC);
    }

    if (rc != SQLITE_OK) {
        throw errors::CacheError{"col" + std::to_string(coln), fmt_loc(loc), rc};
    }
}

int Stmt::clear_bindings() noexcept {
    return sqlite3_clear_bindings(stmt);
}

sqlite3_stmt* Stmt::get() const noexcept {
    return stmt;
}

Vendor Stmt::get_row() noexcept {
    return Vendor{
        get_col<int64_t>(0),
        get_col<std::string>(1),
        get_col<bool>(2),
        get_col<Registry>(3),
        get_col<std::string>(4),
    };
}

void Stmt::insert_row(const Vendor& v, const std::source_location loc) {
    bind(1, v.mac_prefix, loc);
    bind(2, v.vendor_name, loc);
    bind(3, v.is_private, loc);
    bind(4, v.block_type, loc);
    bind(5, v.last_update, loc);

    if (const int rc = step(); rc != SQLITE_DONE) {
        throw errors::CacheError{"step", __func__, rc};
    }
    if (const int rc = reset(); rc != SQLITE_OK) {
        throw errors::CacheError{"reset", __func__, rc};
    }
}

int Stmt::reset() noexcept {
    return sqlite3_reset(stmt);
}

int Stmt::step() noexcept {
    return sqlite3_step(stmt);
}

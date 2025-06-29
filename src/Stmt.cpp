#include <cassert>

#include "Stmt.hpp"

Stmt::Stmt(sqlite3* const conn, const char* str_stmt)
    : prepare_rc{sqlite3_prepare_v2(conn, str_stmt, -1, &stmt, nullptr)} {}

Stmt::Stmt(sqlite3* const conn, const std::string& str_stmt)
    : prepare_rc{sqlite3_prepare_v2(conn, str_stmt.c_str(), -1, &stmt, nullptr)} {}

Stmt::~Stmt() {
    [[maybe_unused]] const int code = sqlite3_finalize(stmt);
    assert(code == SQLITE_OK || code == SQLITE_NOTADB);
}

int Stmt::bind(const int coln, const int64_t value) const noexcept {
    return sqlite3_bind_int64(stmt, coln, value);
}

int Stmt::bind(const int coln, const bool value) const noexcept {
    return sqlite3_bind_int(stmt, coln, value ? 1 : 0);
}

int Stmt::bind(const int coln, const std::string& value) const noexcept {
    return sqlite3_bind_text(stmt, coln, value.c_str(), -1, SQLITE_STATIC);
}

sqlite3_stmt* Stmt::get() const noexcept {
    return stmt;
}

bool Stmt::good() const noexcept {
    return prepare_rc == SQLITE_OK;
}

int Stmt::rc() const noexcept {
    return prepare_rc;
}

int Stmt::reset() const noexcept {
    return sqlite3_reset(stmt);
}

int Stmt::step() const noexcept {
    return sqlite3_step(stmt);
}

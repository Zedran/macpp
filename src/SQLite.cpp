#include <cassert>

#include "SQLite.hpp"

Stmt::Stmt(sqlite3* const conn, const char* str_stmt)
    : prepare_rc{sqlite3_prepare_v2(conn, str_stmt, -1, &stmt, nullptr)} {}

Stmt::Stmt(sqlite3* const conn, const std::string& str_stmt)
    : prepare_rc{sqlite3_prepare_v2(conn, str_stmt.c_str(), -1, &stmt, nullptr)} {}

Stmt::~Stmt() {
    [[maybe_unused]] int code = sqlite3_finalize(stmt);
    assert(code == SQLITE_OK || code == SQLITE_NOTADB);
}

sqlite3_stmt* Stmt::get() const noexcept {
    return stmt;
}

bool Stmt::ok() const noexcept {
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

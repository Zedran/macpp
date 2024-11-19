#include <sqlite3.h>
#include <string>

#include "AppError.hpp"
#include "download.hpp"
#include "utils.hpp"

void create_cache(std::string path) {
    std::vector<Vendor> vendors = download_data();

    sqlite3* conn;
    char*    err;
    int      fin_code{};

    auto free_err = finally([&] { free(err); });

    if (sqlite3_open(path.c_str(), &conn) != SQLITE_OK) {
        throw AppError("failed to open the database");
    }

    auto close = finally([&] { fin_code = sqlite3_close(conn); });

    const char* create_table_stmt =
        R"(CREATE TABLE vendors (
            id      INTEGER PRIMARY KEY,
            addr    TEXT NOT NULL,
            name    TEXT NOT NULL,
            private BOOLEAN,
            block   TEXT NOT NULL,
            updated TEXT NOT NULL
        );)";

    if (sqlite3_exec(conn, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK) {
        throw(AppError(err));
    }

    if (sqlite3_exec(conn, create_table_stmt, nullptr, nullptr, &err) != SQLITE_OK) {
        throw AppError(err);
    }

    const char* insert_stmt =
        "INSERT INTO vendors (id, addr, name, private, block, updated) VALUES (?1, ?2, ?3, ?4, ?5, ?6)";

    sqlite3_stmt* stmt;
    auto          finalize = finally([&] { sqlite3_finalize(stmt); });

    if (sqlite3_prepare_v2(conn, insert_stmt, -1, &stmt, nullptr) != SQLITE_OK) {
        throw AppError("prepare statement failed: " + std::string(sqlite3_errmsg(conn)));
    }

    for (auto& v : vendors) {
        if (v.bind(stmt) != SQLITE_OK) {
            throw AppError("value bind failed: " + std::string(sqlite3_errmsg(conn)));
        }

        if ((fin_code = sqlite3_step(stmt)) != SQLITE_DONE) {
            throw AppError("step failed: " + std::string(sqlite3_errmsg(conn)));
        }

        if (sqlite3_reset(stmt) != SQLITE_OK) {
            throw AppError("reset failed: " + std::string(sqlite3_errmsg(conn)));
        }
    }

    if (sqlite3_exec(conn, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        throw AppError(err);
    }
}

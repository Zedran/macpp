#include <fstream>
#include <sqlite3.h>
#include <string>

#include "AppError.hpp"
#include "cache.hpp"
#include "download.hpp"
#include "utils.hpp"

void create_cache(sqlite3* conn) {
    std::vector<Vendor> vendors = download_data();

    char* err;

    auto free_err = finally([&] { sqlite3_free(err); });

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
        throw AppError("prepare statement failed", conn);
    }

    for (auto& v : vendors) {
        if (v.bind(stmt) != SQLITE_OK) {
            throw AppError("value bind failed", conn);
        }

        if ((sqlite3_step(stmt)) != SQLITE_DONE) {
            throw AppError("step failed", conn);
        }

        if (sqlite3_reset(stmt) != SQLITE_OK) {
            throw AppError("reset failed", conn);
        }
    }

    if (sqlite3_exec(conn, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        throw AppError(err);
    }
}

// Returns sqlite3 connection to cached database. If it does not exist,
// a new cache file is created.
void get_conn(sqlite3*& conn) {
    const std::string  path{"mac.db"};
    const std::fstream cache_file(path);

    bool file_ok = cache_file.good();

    if (sqlite3_open(path.c_str(), &conn) != SQLITE_OK) {
        throw AppError("failed to open the database");
    }

    if (!file_ok) {
        create_cache(conn);
    }
}

std::vector<Vendor> query_addr(sqlite3* conn, const std::string address) {
    const std::vector<int64_t> queries     = construct_queries(address);
    const std::string          stmt_string = build_query_by_id_stmt(queries.size());

    std::vector<Vendor> results;

    sqlite3_stmt* stmt;
    auto          finalize = finally([&] { sqlite3_finalize(stmt); });

    if (sqlite3_prepare_v2(conn, stmt_string.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw(AppError("prepare statement failed", conn));
    }

    for (size_t i = 0; i < queries.size(); i++) {
        if (sqlite3_bind_int64(stmt, static_cast<int>(i + 1), queries[i]) != SQLITE_OK) {
            throw(AppError("value bind failed", conn));
        }
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(Vendor(stmt));
    }

    return results;
}

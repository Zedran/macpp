#include <filesystem>
#include <fstream>
#include <sqlite3.h>
#include <sstream>
#include <string>

#include "FinalAction.hpp"
#include "Vendor.hpp"
#include "cache.hpp"
#include "download.hpp"
#include "exception.hpp"
#include "utils.hpp"

void create_cache(sqlite3* const conn, const std::string& update_fpath) {
    std::unique_ptr<std::iostream> stream;

    if (!update_fpath.empty()) {
        stream = std::make_unique<std::fstream>(get_local_file(update_fpath));
    } else {
        stream = std::make_unique<std::stringstream>(download_data());
    }

    constexpr const char* create_table_stmt =
        R"(CREATE TABLE vendors (
            id      INTEGER PRIMARY KEY,
            addr    TEXT NOT NULL,
            name    TEXT NOT NULL,
            private BOOLEAN,
            block   TEXT NOT NULL,
            updated TEXT NOT NULL
        );)";

    if (sqlite3_exec(conn, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw errors::ExecError.wrap(conn);
    }

    if (sqlite3_exec(conn, create_table_stmt, nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw errors::ExecError.wrap(conn);
    }

    constexpr const char* insert_stmt =
        "INSERT INTO vendors (id, addr, name, private, block, updated) VALUES (?1, ?2, ?3, ?4, ?5, ?6)";

    sqlite3_stmt* stmt{};
    const auto    finalize = finally([&] { sqlite3_finalize(stmt); });

    if (sqlite3_prepare_v2(conn, insert_stmt, -1, &stmt, nullptr) != SQLITE_OK) {
        throw errors::PrepareError.wrap(conn);
    }

    std::string line;

    // More than twice the length of the longest CSV line.
    constexpr size_t MAX_LINE_LENGTH = 300;

    // Discard the header line
    std::getline(*stream, line);

    int code;

    while (std::getline(*stream, line)) {
        if (line.empty() || line.length() > MAX_LINE_LENGTH) {
            continue;
        }
        Vendor v(line);
        v.bind(stmt);

        if (code = sqlite3_step(stmt); code != SQLITE_DONE) {
            throw errors::StepError.wrap(code);
        }

        if (code = sqlite3_reset(stmt); code != SQLITE_OK) {
            throw errors::ResetError.wrap(code);
        }
    }

    if (sqlite3_exec(conn, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        throw errors::ExecError.wrap(conn);
    }
}

void get_conn(sqlite3*& conn, const std::string& cache_path, const std::string& update_fpath) {
    if (int code = sqlite3_open(cache_path.c_str(), &conn); code != SQLITE_OK) {
        throw errors::CacheOpenError.wrap(conn);
    }

    sqlite3_stmt* stmt{};
    const auto    finalize = finally([&] { sqlite3_finalize(stmt); });

    if (sqlite3_prepare_v2(conn, "PRAGMA schema_version", -1, &stmt, nullptr) != SQLITE_OK) {
        throw errors::PrepareError.wrap(conn);
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        // File is not a database and is not empty
        throw errors::NotCacheError;
    }

    int version{-1};

    version = sqlite3_column_int(stmt, 0);

    if (version == 0) {
        // File is empty
        create_cache(conn, update_fpath);
    }
}

std::vector<Vendor> query_addr(sqlite3* const conn, const std::string& address) {
    const std::string stripped_address = remove_addr_separators(address);

    if (stripped_address.empty()) {
        throw errors::EmptyAddrError;
    }

    const std::vector<int64_t> queries     = construct_queries(stripped_address);
    const std::string          stmt_string = build_query_by_id_stmt(queries.size());

    std::vector<Vendor> results;

    sqlite3_stmt* stmt{};
    const auto    finalize = finally([&] { sqlite3_finalize(stmt); });

    if (sqlite3_prepare_v2(conn, stmt_string.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw errors::PrepareError.wrap(conn);
    }

    int code;
    for (size_t i = 0; i < queries.size(); i++) {
        if (code = sqlite3_bind_int64(stmt, static_cast<int>(i + 1), queries[i]); code != SQLITE_OK) {
            throw errors::BindError.wrap(code);
        }
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(Vendor(stmt));
    }

    return results;
}

std::vector<Vendor> query_name(sqlite3* const conn, const std::string& vendor_name) {
    constexpr const char* stmt_string =
        "SELECT * FROM vendors WHERE name LIKE '%' || ?1 || '%' COLLATE BINARY ESCAPE '\\'";

    if (vendor_name.empty()) {
        throw errors::EmptyNameError;
    }

    const std::string query = suppress_like_wildcards(vendor_name);

    std::vector<Vendor> results;

    sqlite3_stmt* stmt{};
    const auto    finalize = finally([&] { sqlite3_finalize(stmt); });

    if (sqlite3_prepare_v2(conn, stmt_string, -1, &stmt, nullptr) != SQLITE_OK) {
        throw errors::PrepareError.wrap(conn);
    }

    if (int code = sqlite3_bind_text(stmt, 1, query.c_str(), -1, SQLITE_STATIC); code != SQLITE_OK) {
        throw errors::BindError.wrap(code);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(Vendor(stmt));
    }

    return results;
}

void update_cache(sqlite3*& conn, const std::string& cache_path, const std::string& update_fpath) {
    using std::filesystem::exists;
    using std::filesystem::file_size;

    if (!exists(cache_path) || file_size(cache_path) == 0) {
        // If a previous database creation run encountered problems,
        // an empty file may remain at cache_path. It is safe to write
        // into such file.
        get_conn(conn, cache_path, update_fpath);
        return;
    }

    const std::string old_cache_path = cache_path + ".old";

    if (exists(old_cache_path)) {
        std::filesystem::remove(old_cache_path.c_str());
    }

    std::filesystem::rename(cache_path.c_str(), old_cache_path.c_str());

    try {
        // Nested try-catch to be able to revert in case of problems
        get_conn(conn, cache_path, update_fpath);
    } catch (std::exception& e) {
        // Restore old cache file if something goes wrong
        if (exists(cache_path)) {
            std::filesystem::remove(cache_path.c_str());
        }

        if (exists(old_cache_path)) {
            std::filesystem::rename(old_cache_path.c_str(), cache_path.c_str());
        }
        throw errors::UpdateError.wrap(e.what());
    }
}

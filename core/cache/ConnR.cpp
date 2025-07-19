#include "cache/ConnR.hpp"
#include "cache/Stmt.hpp"
#include "exception.hpp"
#include "utils.hpp"

std::once_flag ConnR::db_checked{};

ConnR::ConnR() noexcept : Conn{} {}

ConnR::ConnR(const std::string& path, const bool override_once_flags)
    : Conn{path, SQLITE_OPEN_READONLY},
      override_once_flags{override_once_flags} {
    if (sqlite_open_rc != SQLITE_OK) {
        throw errors::CacheError{"open", __func__, sqlite_open_rc};
    }

    if (!override_once_flags) [[likely]] {
        std::call_once(db_checked, [&] { check(); });
    } else [[unlikely]] {
        check();
    }
}

void ConnR::check() const {
    const Stmt stmt{conn, "PRAGMA schema_version"};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, stmt.rc()};
    }

    if (const int rc = stmt.step(); rc == SQLITE_NOTADB) {
        // File is not a database and is not empty
        throw errors::CacheError{"not a cache file", __func__, rc};
    }

    if (!has_table()) {
        throw errors::CacheError{"missing cache data - table not found"};
    }

    if (count_records() == 0) {
        throw errors::CacheError{"missing cache data - no records found"};
    }
}

int64_t ConnR::count_records() const {
    const Stmt stmt{conn, "SELECT COUNT(*) FROM vendors"};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, stmt.rc()};
    }

    if (int rc = stmt.step(); rc != SQLITE_ROW) {
        throw errors::CacheError("count", __func__, rc);
    }

    return stmt.get_col<int64_t>(0);
}

std::vector<Vendor> ConnR::find_by_addr(const std::string& addr) const {
    const std::string stripped_address = remove_addr_separators(addr);

    if (stripped_address.empty()) {
        throw errors::Error{"empty MAC address"};
    }

    const std::vector<int64_t> queries     = construct_queries(stripped_address);
    const std::string          stmt_string = build_query_by_id_stmt(queries.size());

    std::vector<Vendor> results;

    const Stmt stmt{conn, stmt_string};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, conn};
    }

    int rc;
    for (size_t i = 0; i < queries.size(); i++) {
        if (rc = stmt.bind(static_cast<int>(i + 1), queries[i]); rc != SQLITE_OK) {
            throw errors::CacheError{"bind", __func__, rc};
        }
    }

    while (stmt.step() == SQLITE_ROW) {
        results.emplace_back(Vendor{stmt});
    }

    return results;
}

std::vector<Vendor> ConnR::find_by_name(const std::string& name) const {
    constexpr const char* stmt_string =
        "SELECT * FROM vendors "
        "WHERE name LIKE '%' || ?1 || '%' COLLATE BINARY ESCAPE '\\'";

    if (name.empty()) {
        throw errors::Error{"empty vendor name"};
    }

    const std::string query = suppress_like_wildcards(name);

    std::vector<Vendor> results;

    const Stmt stmt{conn, stmt_string};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, conn};
    }

    if (int rc = stmt.bind(1, query); rc != SQLITE_OK) {
        throw errors::CacheError{"bind", __func__, rc};
    }

    while (stmt.step() == SQLITE_ROW) {
        results.emplace_back(Vendor{stmt});
    }

    return results;
}

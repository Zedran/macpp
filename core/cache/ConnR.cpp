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

ConnR::~ConnR() = default;

void ConnR::check() const {
    if (version() != EXPECTED_CACHE_VERSION) {
        throw errors::CacheError{"database version mismatch, update required"};
    }

    if (!has_table()) {
        throw errors::CacheError{"missing cache data - table not found"};
    }

    if (count_records() == 0) {
        throw errors::CacheError{"missing cache data - no records found"};
    }
}

int64_t ConnR::count_records() const {
    Stmt stmt{conn, "SELECT COUNT(*) FROM vendors"};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, stmt.rc()};
    }

    if (int rc = stmt.step(); rc != SQLITE_ROW) {
        throw errors::CacheError("count", __func__, rc);
    }

    return stmt.get_col<int64_t>(0);
}

std::vector<Vendor> ConnR::export_records() const {
    Stmt stmt{conn, "SELECT * FROM vendors"};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, stmt.rc()};
    }

    std::vector<Vendor> results;

    while (stmt.step() == SQLITE_ROW) {
        results.emplace_back(stmt.get_row());
    }

    return results;
}

std::vector<Vendor> ConnR::find_by_addr(const std::string& addr) const {
    const std::string stripped_address = remove_addr_separators(addr);

    if (stripped_address.empty()) {
        throw errors::Error{"empty MAC address"};
    }

    const std::vector<int64_t> queries     = construct_queries(stripped_address);
    const std::string          stmt_string = build_find_by_addr_stmt(queries.size());

    Stmt stmt{conn, stmt_string};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, conn};
    }

    int rc;
    for (size_t i = 0; i < queries.size(); i++) {
        if (rc = stmt.bind(static_cast<int>(i + 1), queries[i]); rc != SQLITE_OK) {
            throw errors::CacheError{"bind", __func__, rc};
        }
    }

    std::vector<Vendor> results;

    while (stmt.step() == SQLITE_ROW) {
        results.emplace_back(stmt.get_row());
    }

    return results;
}

std::set<Vendor> ConnR::find_by_name(std::span<const std::string> names) const {
    constexpr const char* stmt_string =
        "SELECT * FROM vendors "
        "WHERE name LIKE '%' || ?1 || '%' COLLATE NOCASE ESCAPE '\\'";

    if (names.empty()) {
        throw errors::Error{"no vendor names provided"};
    }

    Stmt stmt{conn, stmt_string};
    if (!stmt) {
        throw errors::CacheError{"prepare", __func__, conn};
    }

    std::set<Vendor> results;

    for (const auto& vn : names) {
        if (vn.empty()) {
            throw errors::Error{"empty vendor name encountered"};
        }

        if (int rc = stmt.bind(1, vn); rc != SQLITE_OK) {
            throw errors::CacheError{"bind", __func__, rc};
        }

        while (stmt.step() == SQLITE_ROW) {
            results.emplace(stmt.get_row());
        }

        if (int rc = stmt.clear_bindings(); rc != SQLITE_OK) {
            throw errors::CacheError{"clear_bindings", __func__, rc};
        }

        if (int rc = stmt.reset(); rc != SQLITE_OK) {
            throw errors::CacheError{"reset", __func__, rc};
        }
    }

    return results;
}

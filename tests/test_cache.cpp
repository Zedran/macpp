#include <catch2/catch_test_macros.hpp>
#include <format>
#include <sqlite3.h>

#include "FinalAction.hpp"
#include "cache.hpp"

// Creates an in-memory database for testing cache. Returns sqlite3 code.
int create_test_cache(sqlite3*& conn) {
    const char* create_table_stmt =
        R"(CREATE TABLE vendors (
            id      INTEGER PRIMARY KEY,
            addr    TEXT NOT NULL,
            name    TEXT NOT NULL,
            private BOOLEAN,
            block   TEXT NOT NULL,
            updated TEXT NOT NULL
        );
        INSERT INTO vendors (id, addr, name, private, block, updated)
        VALUES
        (12, '00:00:0C', 'Cisco Systems, Inc', false, 'MA-L', '2015/11/17'),
        (12876445, '0C:47:A9:D', 'DIG_LINK', false, 'MA-M', '2024/10/28');
    )";

    int code = -1;

    if ((code = sqlite3_open(":memory:", &conn)) != SQLITE_OK)
        return code;

    if ((code = sqlite3_exec(conn, create_table_stmt, nullptr, nullptr, nullptr)) != SQLITE_OK)
        return code;

    return code;
}

TEST_CASE("injections") {
    sqlite3_initialize();

    sqlite3* conn{};

    auto cleanup = finally([&] {
        sqlite3_close(conn);
        sqlite3_shutdown();
    });

    int code;

    if ((code = create_test_cache(conn)) != SQLITE_OK) {
        FAIL(std::format(
            "create_test_cache failed: ({}) {}",
            code,
            static_cast<std::string>(sqlite3_errmsg(conn))
        ));
    }

    const std::string cases[] = {
        "DIG_LINK",
    };

    for (const auto& c : cases) {
        std::vector out = query_name(conn, c);

        if (out.size() != 1)
            FAIL(std::format("failed for case '{}': vector length {}", c, out.size()));

        if (c != out.at(0).vendor_name)
            FAIL(std::format("failed for case '{}': got '{}', expected '{}'", c, out.at(0).vendor_name, c));
    }

    const std::string throw_cases[] = {
        "",
    };

    for (const auto& c : throw_cases) {
        REQUIRE_THROWS(query_name(conn, c));
    }
}

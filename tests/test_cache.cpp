#include <catch2/catch_test_macros.hpp>
#include <format>
#include <sqlite3.h>

#include "FinalAction.hpp"
#include "cache.hpp"

TEST_CASE("get_conn") {
    sqlite3_initialize();

    sqlite3* conn{};

    auto cleanup = finally([&] {
        sqlite3_close(conn);
        sqlite3_shutdown();
    });

    // A good cache file
    REQUIRE_NOTHROW(get_conn(conn, "testdata/sample.db"));
    REQUIRE(sqlite3_close(conn) == SQLITE_OK);

    // Non-empty file that is not a SQLite database
    REQUIRE_THROWS(get_conn(conn, "testdata/not_cache.txt"));
}

TEST_CASE("injections") {
    sqlite3_initialize();

    sqlite3* conn{};

    auto cleanup = finally([&] {
        sqlite3_close(conn);
        sqlite3_shutdown();
    });

    get_conn(conn, "testdata/sample.db");

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

TEST_CASE("query_addr") {
    sqlite3_initialize();

    sqlite3* conn{};

    auto cleanup = finally([&] {
        sqlite3_close(conn);
        sqlite3_shutdown();
    });

    get_conn(conn, "testdata/sample.db");

    Vendor expected{"00:00:0C", "Cisco Systems, Inc", false, "MA-L", "2015/11/17"};

    const std::string cases[] = {
        "00:00:0C",          // with separators, short
        "00:00:0C:12:3",     // with separators, partial
        "00:00:0C:12:34:56", // with separators, full
        "00000C",            // no separators, short
        "00000C123",         // no separators, partial
        "00000C123456",      // no separators, full
        "00:::00::::0C",     // many separators in row
        "00000c",            // lower-case hex, no separators
        "00:00:0c:12:34:56", // lower case hex, separators
    };

    std::vector<Vendor> results;

    for (auto c : cases) {
        CAPTURE(c);

        results = query_addr(conn, c);

        REQUIRE(results.size() == 1);

        Vendor& out = results.at(0);

        REQUIRE(out.mac_prefix == expected.mac_prefix);
        REQUIRE(out.vendor_name == expected.vendor_name);
        REQUIRE(out.is_private == expected.is_private);
        REQUIRE(out.block_type == expected.block_type);
        REQUIRE(out.last_update == expected.last_update);

        results.pop_back();
    }

    // Valid, not found
    results = query_addr(conn, "012345");
    REQUIRE(results.empty());

    const std::string throw_cases[] = {
        "",             // empty
        "0000c",        // too short
        "0c",           // too short
        "c",            // too short
        "::::::::::::", // separators do not count
        "01234x",       // non-number
    };

    for (auto c : throw_cases) {
        CAPTURE(c);
        REQUIRE_THROWS(query_addr(conn, c));
    }
}

TEST_CASE("query_name") {
    sqlite3_initialize();

    sqlite3* conn{};

    auto cleanup = finally([&] {
        sqlite3_close(conn);
        sqlite3_shutdown();
    });

    get_conn(conn, "testdata/sample.db");

    Vendor expected{"00:00:0C", "Cisco Systems, Inc", false, "MA-L", "2015/11/17"};

    const std::string cases[] = {
        "Cisco Systems, Inc",
        "cisco systems, inc",
        "cisco sys",
        "Cisco",
        "cisco",
        "Systems",
        "systems",
        "CiScO SYS",
    };

    std::vector<Vendor> results;

    for (auto c : cases) {
        CAPTURE(c);

        results = query_name(conn, c);

        REQUIRE(results.size() == 1);

        Vendor& out = results.at(0);

        REQUIRE(out.mac_prefix == expected.mac_prefix);
        REQUIRE(out.vendor_name == expected.vendor_name);
        REQUIRE(out.is_private == expected.is_private);
        REQUIRE(out.block_type == expected.block_type);
        REQUIRE(out.last_update == expected.last_update);

        results.pop_back();
    }

    // Valid, not found
    results = query_name(conn, "non-existent");
    REQUIRE(results.empty());

    const std::string throw_cases[] = {
        "", // empty
    };

    for (auto c : throw_cases) {
        CAPTURE(c);
        REQUIRE_THROWS(query_addr(conn, c));
    }
}

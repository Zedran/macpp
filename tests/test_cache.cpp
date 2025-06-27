#include <catch2/catch_test_macros.hpp>
#include <format>
#include <map>
#include <sqlite3.h>

#include "FinalAction.hpp"
#include "SQLite.hpp"
#include "cache.hpp"
#include "exception.hpp"

// Tests the create_cache function using a local file.
TEST_CASE("create_cache") {
    const std::vector<Vendor> cases = {
        Vendor{"00:00:0C", "Cisco Systems, Inc", false, "MA-L", "2015/11/17"},
        Vendor{"00:48:54", "", true, "", ""},
    };

    sqlite3_initialize();

    sqlite3* conn{};

    const auto cleanup = finally([&] {
        sqlite3_close(conn);
        sqlite3_shutdown();
    });

    REQUIRE_NOTHROW(get_conn(conn, ":memory:", "testdata/update.csv"));

    const Stmt stmt{conn, "SELECT * FROM vendors"};
    REQUIRE(stmt.ok());

    std::vector<Vendor> out;

    while (stmt.step() == SQLITE_ROW) {
        out.push_back(Vendor(stmt.get()));
    }

    REQUIRE(cases.size() == out.size());

    for (auto& o : out) {
        bool found = false;

        for (auto& c : cases) {
            if (o.mac_prefix == c.mac_prefix) {
                found = true;

                REQUIRE(o.vendor_name == c.vendor_name);
                REQUIRE(o.is_private == c.is_private);
                REQUIRE(o.block_type == c.block_type);
                REQUIRE(o.last_update == c.last_update);

                break;
            }
        }
        REQUIRE(found == true);
    }

    // Line length limits
    sqlite3* conn2{};

    const auto cleanup2 = finally([&] { sqlite3_close(conn2); });

    REQUIRE_NOTHROW(update_cache(conn2, ":memory:", "testdata/poisoned.csv"));

    const Stmt stmt2{conn2, "SELECT * FROM vendors"};
    REQUIRE(stmt2.ok());

    out.clear();
    while (stmt2.step() == SQLITE_ROW) {
        out.push_back(Vendor(stmt2.get()));
    }

    REQUIRE(out.size() == 1);
    CAPTURE(out[0]);
    REQUIRE(out[0].vendor_name == "Cisco Systems, Inc");
}

TEST_CASE("get_conn") {
    sqlite3_initialize();

    sqlite3* conn{};

    const auto cleanup = finally([&] {
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

    const auto cleanup = finally([&] {
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

    const std::map<const std::string, const errors::Error> throw_cases = {
        {"", errors::Error{"empty vendor name"}},
    };

    for (const auto& [input, expected_error] : throw_cases) {
        CAPTURE(input);

        try {
            query_name(conn, input);
        } catch (const errors::Error& e) {
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL(std::format("unexpected exception was thrown: '{}'", e.what()));
        }

        FAIL("no exception was thrown");
    }
}

TEST_CASE("query_addr") {
    sqlite3_initialize();

    sqlite3* conn{};

    const auto cleanup = finally([&] {
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

    const auto empty     = errors::Error{"empty MAC address"};
    const auto too_short = errors::Error{"specified MAC address is too short"};
    const auto invalid   = errors::Error{"specified MAC address contains invalid characters"};

    const std::map<const std::string, const errors::Error&> throw_cases = {
        {"", empty},             // empty
        {"0000c", too_short},    // too short
        {"0c", too_short},       // too short
        {"c", too_short},        // too short
        {"::::::::::::", empty}, // separators do not count
        {"01234x", invalid},     // non-number
    };

    for (const auto& [input, expected_error] : throw_cases) {
        CAPTURE(input);

        try {
            query_addr(conn, input);
        } catch (const errors::Error& e) {
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL(std::format("unexpected exception was thrown: '{}'", e.what()));
        }

        FAIL("no exception was thrown");
    }
}

TEST_CASE("query_name") {
    sqlite3_initialize();

    sqlite3* conn{};

    const auto cleanup = finally([&] {
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

    const std::map<const std::string, const errors::Error> throw_cases = {
        {"", errors::Error{"empty vendor name"}},
    };

    for (const auto& [input, expected_error] : throw_cases) {
        CAPTURE(input);

        try {
            query_name(conn, input);
        } catch (const errors::Error& e) {
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL(std::format("unexpected exception was thrown: '{}'", e.what()));
        }

        FAIL("no exception was thrown");
    }
}

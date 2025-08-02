#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <map>
#include <sqlite3.h>

#include "cache/ConnR.hpp"
#include "cache/ConnRW.hpp"
#include "cache/Stmt.hpp"
#include "exception.hpp"

// Tests the ability of ConnRW class to correctly insert records into the cache
// using a local file.
TEST_CASE("ConnRW::insert") {
    const std::vector<Vendor> cases = {
        Vendor{0x00000C, "Cisco Systems, Inc", Registry::MA_L, "2015/11/17"},
        Vendor{0x004854, "", Registry::Unknown, ""},
        Vendor{0x8C1F64FFC, "Invendis Technologies India Pvt Ltd", Registry::MA_S, "2022/07/19"},
    };

    ConnRW conn_rw{"file:memdb_connrw_insert?mode=memory&cache=shared", true};

    std::ifstream good_file{"testdata/update.csv"};

    REQUIRE_NOTHROW(conn_rw.insert(good_file));

    const Stmt stmt{conn_rw.get(), "SELECT * FROM vendors"};
    REQUIRE(stmt.rc() == SQLITE_OK);

    std::vector<Vendor> out;

    while (stmt.step() == SQLITE_ROW) {
        out.emplace_back(stmt.get_row());
    }

    REQUIRE(cases.size() == out.size());

    for (const auto& o : out) {
        bool found = false;

        for (const auto& c : cases) {
            if (o.mac_prefix == c.mac_prefix) {
                found = true;

                REQUIRE(o.vendor_name == c.vendor_name);
                REQUIRE(o.block_type == c.block_type);
                REQUIRE(o.last_update == c.last_update);

                break;
            }
        }
        REQUIRE(found == true);
    }

    REQUIRE(conn_rw.clear_table() == SQLITE_OK);
    REQUIRE(stmt.reset() == SQLITE_OK);
    out.clear();

    // Line length limits

    std::ifstream poisoned_file{"testdata/poisoned.csv"};

    REQUIRE_NOTHROW(conn_rw.insert(poisoned_file));

    while (stmt.step() == SQLITE_ROW) {
        out.emplace_back(stmt.get_row());
    }

    REQUIRE(out.size() == 1);
    CAPTURE(out[0]);
    REQUIRE(out[0].vendor_name == "Cisco Systems, Inc");
}

TEST_CASE("ConnR construction") {
    // Non-empty file that is not a SQLite database
    REQUIRE_THROWS(ConnR{"testdata/not_cache.txt"}, true);

    // A good cache file
    REQUIRE_NOTHROW(ConnR{"testdata/sample.db"}, true);

    const std::string db_path = "file:connr_construction_empty?mode=memory&cache=shared";

    // Opens the database and creates an empty table
    const ConnRW conn_rw{db_path, true};

    // Throws, because there are no records in the table
    REQUIRE_THROWS(ConnR{db_path, true});

    const Stmt stmt{conn_rw.get(), "DROP TABLE vendors"};
    REQUIRE(stmt.good());
    REQUIRE(stmt.step() == SQLITE_DONE);

    // Throws, because there is no table
    REQUIRE_THROWS(ConnR{db_path, true});

    // Throws, because the file does not exist
    REQUIRE_THROWS(ConnR{"non-existent", true});
}

TEST_CASE("ConnRW construction") {
    // Non-empty file that is not a SQLite database
    REQUIRE_THROWS(ConnRW{"testdata/not_cache.txt", true});

    // A good cache file
    REQUIRE_NOTHROW(ConnRW{"testdata/sample.db", true});

    // An in-memory, empty file
    REQUIRE_NOTHROW(ConnRW{":memory:", true});
}

// Tests the behaviour of ConnRW destructor.
//
// This test uses ConnRW::insert(istream&) member function.
// It begins new transaction, parses the CSV file stream, inserts records
// into the database and commits changes on success, but does not close
// the transaction if CSV parser throws.
//
// If a transaction begins and CSV parser throws an exception causing ConnRW
// object to leave scope, the destructor should roll changes back automatically,
// because the transaction is not closed on failure.
//
// If the stream is processed in its entirety without triggering an exception,
// the transaction should be committed, so that changes persist across scopes.
TEST_CASE("ConnRW destruction") {
    const std::string db_path = "file:conn_rw_rollback?mode=memory&cache=shared";

    // Keeps the database alive across scopes.
    const ConnRW conn_master{db_path, true};

    const Stmt stmt{conn_master.get(), "SELECT COUNT(*) FROM vendors"};
    REQUIRE(stmt.rc() == SQLITE_OK);

    REQUIRE(stmt.step() == SQLITE_ROW);
    REQUIRE(stmt.get_col<int>(0) == 0);
    REQUIRE(stmt.reset() == SQLITE_OK);

    {
        // Correctly structured file causes transaction to be committed
        // into the database.
        ConnRW        conn_rw_good{db_path, true};
        std::ifstream good_file{"testdata/update.csv"};
        REQUIRE_NOTHROW(conn_rw_good.insert(good_file));
    }

    REQUIRE(stmt.step() == SQLITE_ROW);
    REQUIRE(stmt.get_col<int>(0) == 3);
    REQUIRE(stmt.reset() == SQLITE_OK);

    REQUIRE(conn_master.clear_table() == SQLITE_OK);

    {
        // Incorrectly structured file causes current transaction to be
        // rolled back as the object is destroyed.
        ConnRW        conn_rw_failing{db_path, true};
        std::ifstream malformed_file{"testdata/malformed.csv"};
        REQUIRE_THROWS_AS(conn_rw_failing.insert(malformed_file), errors::QuotedTermSeqError);
    }

    REQUIRE(stmt.step() == SQLITE_ROW);

    REQUIRE(stmt.get_col<int>(0) == 0);
}

TEST_CASE("injections") {
    const ConnR conn{"testdata/sample.db", true};

    const std::string cases[] = {
        "DIG_LINK",
    };

    for (const auto& c : cases) {
        std::vector out = conn.find_by_name(c);

        REQUIRE(out.size() == 1);
        REQUIRE(c == out[0].vendor_name);
    }

    const std::map<const std::string, const errors::Error> throw_cases = {
        {"", errors::Error{"empty vendor name"}},
    };

    for (const auto& [input, expected_error] : throw_cases) {
        CAPTURE(input);

        try {
            conn.find_by_name(input);
        } catch (const errors::Error& e) {
            CAPTURE(e);
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL("unexpected exception was thrown: " + std::string{e.what()} + "'");
        }

        FAIL("no exception was thrown");
    }
}

TEST_CASE("ConnR::find_by_addr") {
    const ConnR conn{"testdata/sample.db", true};

    const Vendor expected{0x00000C, "Cisco Systems, Inc", Registry::MA_L, "2015/11/17"};

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

        results = conn.find_by_addr(c);

        REQUIRE(results.size() == 1);

        Vendor& out = results.at(0);

        REQUIRE(out.mac_prefix == expected.mac_prefix);
        REQUIRE(out.vendor_name == expected.vendor_name);
        REQUIRE(out.block_type == expected.block_type);
        REQUIRE(out.last_update == expected.last_update);

        results.pop_back();
    }

    // Valid, not found
    results = conn.find_by_addr("012345");
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
            conn.find_by_addr(input);
        } catch (const errors::Error& e) {
            CAPTURE(e);
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL("unexpected exception was thrown: " + std::string{e.what()} + "'");
        }

        FAIL("no exception was thrown");
    }
}

TEST_CASE("ConnR::find_by_name") {
    const ConnR conn{"testdata/sample.db", true};

    const Vendor expected{0x00000C, "Cisco Systems, Inc", Registry::MA_L, "2015/11/17"};

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

        results = conn.find_by_name(c);

        REQUIRE(results.size() == 1);

        Vendor& out = results.at(0);

        REQUIRE(out.mac_prefix == expected.mac_prefix);
        REQUIRE(out.vendor_name == expected.vendor_name);
        REQUIRE(out.block_type == expected.block_type);
        REQUIRE(out.last_update == expected.last_update);

        results.pop_back();
    }

    // Valid, not found
    results = conn.find_by_name("non-existent");
    REQUIRE(results.empty());

    const std::map<const std::string, const errors::Error> throw_cases = {
        {"", errors::Error{"empty vendor name"}},
    };

    for (const auto& [input, expected_error] : throw_cases) {
        CAPTURE(input);

        try {
            conn.find_by_name(input);
        } catch (const errors::Error& e) {
            CAPTURE(e);
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL("unexpected exception was thrown: " + std::string{e.what()} + "'");
        }

        FAIL("no exception was thrown");
    }
}

TEST_CASE("user_version") {
    const std::string path = "file:memdb_user_version?mode=memory&cache=shared";

    ConnRW conn_rw{path, true};
    REQUIRE(conn_rw.set_version(0) == SQLITE_OK);

    REQUIRE_THROWS(ConnR{path, true});
}

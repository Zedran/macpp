#include <algorithm>
#include <array>
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
        Vendor{0x00000C, "Cisco Systems, Inc", false, Registry::MA_L, "2015/11/17"},
        Vendor{0x004854, "", true, Registry::Unknown, ""},
        Vendor{0x8C1F64FFC, "Invendis Technologies India Pvt Ltd", false, Registry::MA_S, "2022/07/19"},
    };

    ConnRW conn_rw{"file:memdb_connrw_insert?mode=memory&cache=shared", true};

    std::ifstream good_file{"testdata/update.csv"};

    REQUIRE_NOTHROW(conn_rw.insert(good_file, false));

    Stmt stmt{conn_rw.get(), "SELECT * FROM vendors"};
    REQUIRE(stmt.rc() == SQLITE_OK);

    std::vector<Vendor> out;

    while (stmt.step() == SQLITE_ROW) {
        out.emplace_back(stmt.get_row());
    }

    REQUIRE(cases.size() == out.size());

    for (const auto& o : out) {
        bool found = std::ranges::find(cases, o) != cases.end();
        CAPTURE(o);
        REQUIRE(found);
    }

    REQUIRE(conn_rw.clear_table() == SQLITE_OK);
    REQUIRE(stmt.reset() == SQLITE_OK);
    out.clear();

    // Line length limits

    std::ifstream poisoned_file{"testdata/poisoned.csv"};

    REQUIRE_NOTHROW(conn_rw.insert(poisoned_file, false));

    while (stmt.step() == SQLITE_ROW) {
        out.emplace_back(stmt.get_row());
    }

    REQUIRE(out.size() == 1);
    CAPTURE(out[0]);
    REQUIRE(out[0].vendor_name == "Cisco Systems, Inc");
}

TEST_CASE("ConnRW::customize_db: success") {
    const std::string db_path = "file:connrw_customize_db_success?mode=memory&cache=shared";

    ConnRW conn{db_path, true};

    // <input, entry after database modifications>
    std::map<std::string, Vendor> cases = {
        {"52:54:00,,true,,0001/01/01\n", Vendor{"52:54:00,QEMU/KVM,true,,0001/01/01"}},
        {"08:00:27,PCS Systemtechnik GmbH,false,MA-L,2016/10/30\n", Vendor{"08:00:27,PCS Systemtechnik GmbH (VirtualBox),false,MA-L,2016/10/30"}},
    };

    std::stringstream ss;
    ss << "Header\n";
    for (const auto& [c, _] : cases) {
        ss << c;
    }

    std::stringstream cerr_capture;
    REQUIRE_NOTHROW(conn.insert(ss, true, cerr_capture));

    std::string line;

    std::vector<std::string> err_messages;

    while (std::getline(cerr_capture, line)) {
        err_messages.push_back(line);
    }

    CAPTURE(err_messages);
    REQUIRE(err_messages.empty());

    Stmt stmt{conn.get(), "SELECT * FROM vendors"};
    REQUIRE(stmt.rc() == SQLITE_OK);

    std::vector<Vendor> out;

    while (stmt.step() == SQLITE_ROW) {
        out.emplace_back(stmt.get_row());
    }

    CAPTURE(out);
    REQUIRE(out.size() == 3);

    cases.emplace("inserted entry", Vendor{0x024200, "Docker container interface (02:42)", true, Registry::Unknown, ""});

    for (const auto& [_, c] : cases) {
        bool found = std::ranges::find(out, c) != out.end();
        CAPTURE(c);
        REQUIRE(found);
    }
}

TEST_CASE("ConnRW::customize_db: warnings") {
    const std::string db_path = "file:connrw_customize_db_warnings?mode=memory&cache=shared";

    ConnRW conn{db_path, true};

    std::stringstream ss;
    ss << "Header\n02:42:00,Placeholder,true,,";

    std::stringstream cerr_capture;
    REQUIRE_NOTHROW(conn.insert(ss, true, cerr_capture));

    const std::array<std::string, 3> expected = {
        "[ customize_db ] UPDATE vendors SET name = 'QEMU/KVM' WHERE prefix = 0x525400: unexpected number of changes (0)",
        "[ customize_db ] UPDATE vendors SET name = name || ' (VirtualBox)' WHERE prefix = 0x080027: unexpected number of changes (0)",
        "[ insert_row ] step: (19) constraint failed",
    };

    std::string line;
    while (std::getline(cerr_capture, line)) {
        bool found = std::ranges::find(expected, line) != expected.end();
        CAPTURE(line);
        REQUIRE(found);
    }
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

    Stmt stmt{conn_rw.get(), "DROP TABLE vendors"};
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
    ConnRW conn_master{db_path, true};

    Stmt stmt{conn_master.get(), "SELECT COUNT(*) FROM vendors"};
    REQUIRE(stmt.rc() == SQLITE_OK);

    REQUIRE(stmt.step() == SQLITE_ROW);
    REQUIRE(stmt.get_col<int>(0) == 0);
    REQUIRE(stmt.reset() == SQLITE_OK);

    {
        // Correctly structured file causes transaction to be committed
        // into the database.
        ConnRW        conn_rw_good{db_path, true};
        std::ifstream good_file{"testdata/update.csv"};
        REQUIRE_NOTHROW(conn_rw_good.insert(good_file, false));
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
        REQUIRE_THROWS_AS(conn_rw_failing.insert(malformed_file, false), errors::QuotedTermSeqError);
    }

    REQUIRE(stmt.step() == SQLITE_ROW);

    REQUIRE(stmt.get_col<int>(0) == 0);
}

TEST_CASE("undefined Registry values") {
    const std::vector<Vendor> expected = {
        {0x000000, "", true, Registry::Unknown, ""},
        {0x00000C, "", true, Registry::Unknown, ""},
    };

    const ConnR conn{"testdata/poisoned.db", true};

    std::vector<Vendor> out;

    REQUIRE_NOTHROW(out = conn.export_records());
    REQUIRE(out == expected);
}

TEST_CASE("ConnR::export_records") {
    const ConnR         conn{"testdata/sample.db", true};
    std::vector<Vendor> out = conn.export_records();
    REQUIRE(out.size() == 3);
}

TEST_CASE("ConnR::find_by_addr") {
    const ConnR conn{"testdata/sample.db", true};

    const Vendor expected{0x00000C, "Cisco Systems, Inc", false, Registry::MA_L, "2015/11/17"};

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

        REQUIRE(out == expected);

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

    const Vendor expected{0x00000C, "Cisco Systems, Inc", false, Registry::MA_L, "2015/11/17"};

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

        REQUIRE(out == expected);

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

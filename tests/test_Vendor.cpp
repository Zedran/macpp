#include <catch2/catch_test_macros.hpp>
#include <format>
#include <map>
#include <sstream>

#include "ConnR.hpp"
#include "ConnRW.hpp"
#include "Vendor.hpp"
#include "exception.hpp"

// Tests whether the CSV lines are correctly parsed into a Vendor struct.
// The main challenge is that the file is comma-separated and sometimes
// vendor_name can contain a comma, therefore the CSV string needs to be
// wrapped in quotes. Vendor constructor must account for that when gathering
// values from line.
TEST_CASE("Vendor::Vendor(const std::string& line)") {
    struct test_case {
        std::string input;
        Vendor      expected;
    };

    const test_case cases[] = {
        test_case{
            // Quoted vendor name
            R"(00:00:0C,"Cisco Systems, Inc",false,MA-L,2015/11/17)",
            Vendor{"00:00:0C", "Cisco Systems, Inc", false, "MA-L", "2015/11/17"}
        },
        test_case{
            // Non-quoted vendor name
            R"(00:00:0D,FIBRONICS LTD.,false,MA-L,2015/11/17)",
            Vendor{"00:00:0D", "FIBRONICS LTD.", false, "MA-L", "2015/11/17"}
        },
        test_case{
            // Quoted vendor name, longer prefix
            R"(5C:F2:86:D,"BrightSky, LLC",false,MA-M,2019/07/02)",
            Vendor{"5C:F2:86:D", "BrightSky, LLC", false, "MA-M", "2019/07/02"}
        },
        test_case{
            // Non-quoted vendor name, longer prefix
            R"(8C:1F:64:F5:A,Telco Antennas Pty Ltd,false,MA-S,2021/10/13)",
            Vendor{"8C:1F:64:F5:A", "Telco Antennas Pty Ltd", false, "MA-S", "2021/10/13"}
        },
        test_case{
            // Escaped quotes inside quoted vendor name
            R"(2C:7A:FE,"IEE&E ""Black"" ops",false,MA-L,2010/07/26)",
            Vendor{"2C:7A:FE", "IEE&E \"Black\" ops", false, "MA-L", "2010/07/26"},
        },
        test_case{
            // Private block
            R"(00:48:54,,true,,0001/01/01)",
            Vendor{"00:48:54", "", true, "", ""},
        },
        test_case{
            // The last update field is empty, but valid (comma present)
            R"(00:00:0D,FIBRONICS LTD.,false,MA-L,)",
            Vendor{"00:00:0D", "FIBRONICS LTD.", false, "MA-L", ""},
        },
    };

    for (auto& c : cases) {
        CAPTURE(c.input);
        Vendor out = Vendor(c.input);

        REQUIRE(out.mac_prefix == c.expected.mac_prefix);
        REQUIRE(out.vendor_name == c.expected.vendor_name);
        REQUIRE(out.is_private == c.expected.is_private);
        REQUIRE(out.block_type == c.expected.block_type);
        REQUIRE(out.last_update == c.expected.last_update);
    }

    using namespace errors;

    const std::map<const std::string, const ParsingError> throw_cases = {
        {"", NoCommaError{""}},                                                             // Empty line
        {"00:00:00Vendor name", NoCommaError{""}},                                          // No commas
        {R"(5C:F2:86:D,"BrightSky, LLC,false,MA-M,2019/07/02)", QuotedTermSeqError{""}},    // No closing quote after vendor name
        {R"(5C:F2:86:D,"BrightSky, LLC"false,MA-M,2019/07/02)", QuotedTermSeqError{""}},    // Closing quote after vendor name present, but no comma
        {R"(00:00:00,"IEE&E ""Black"" ops,false,MA-L,2010/07/26)", QuotedTermSeqError{""}}, // No closing quote with escaped quote present
        {R"(00:00:0C,"Cisco Systems, Inc",no,MA-L,2015/11/17)", PrivateInvalidError{""}},   // Invalid private field
        {R"(00:00:0C,"Cisco Systems, Inc",falseMA-L,2015/11/17)", PrivateTermError{""}},    // No comma between private and block type fields
        {R"(00:00:0D,FIBRONICS LTD.)", UnquotedTermError{""}},                              // No comma after vendor name
        {R"(00:00:0D,FIBRONICS LTD.,)", PrivateInvalidError{""}},                           // Comma after vendor name ends the line
        {R"(00:00:0D,FIBRONICS LTD.,false,)", BlockTypeTermError{""}},                      // Comma after private field ends the line
        {R"(00:00:0D,FIBRONICS LTD.,false)", PrivateTermError{""}},                         // No comma after private field
        {R"(00:00:0D,FIBRONICS LTD.,false,MA-L)", BlockTypeTermError{""}},                  // No comma after block type field
        {R"(5C:F2:86:D,"BrightSky, LLC",,MA-M,2019/07/02)", PrivateInvalidError{""}},       // Private field empty
    };

    for (const auto& [input, expected_error] : throw_cases) {
        CAPTURE(input);

        try {
            Vendor v{input};
        } catch (const errors::ParsingError& e) {
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL(std::format("unexpected exception was thrown: '{}'", e.what()));
        }

        FAIL("no exception was thrown");
    }
}

// Ensures that a NULL text value returned from the database
// is correctly handled. Although the vendors table does not
// allow NULL text values, the check was implemented
// for protection against a compromised cache.
TEST_CASE("Vendor::Vendor(sqlite3_stmt* stmt)") {
    const ConnR conn{"testdata/poisoned.db", true};

    std::vector<Vendor> results = conn.find_by_addr("00:00:0C");

    REQUIRE(!results.empty());
    REQUIRE(results.at(0).vendor_name == "");
}

// Ensures that binding and insertion into the table produces
// desired entries.
TEST_CASE("Vendor::bind") {
    const std::string db_path = "file:memdb_vendor_bind?mode=memory&cache=shared";

    ConnRW conn_rw{db_path, true};

    const Vendor cases[] = {
        Vendor{"00:00:0C", "Cisco Systems, Inc", false, "MA-L", "2015/11/17"},
        Vendor{"00:48:54", "", true, "", "0001/01/01"},
    };

    REQUIRE(conn_rw.begin() == SQLITE_OK);

    for (const auto& c : cases) {
        CAPTURE(c.mac_prefix);
        REQUIRE_NOTHROW(conn_rw.insert(c));
    }

    // Empty MAC prefix not allowed
    const Vendor malformed{"", "Cisco Systems, Inc", false, "MA-L", "2015/11/17"};

    REQUIRE_THROWS(conn_rw.insert(malformed));

    REQUIRE(conn_rw.commit() == SQLITE_OK);

    const ConnR conn_r{db_path, true};

    std::vector<Vendor> results;

    for (const auto& c : cases) {
        CAPTURE(c.mac_prefix);

        results = conn_r.find_by_addr(c.mac_prefix);

        REQUIRE(results.size() == 1);

        Vendor& out = results.at(0);

        REQUIRE(out.mac_prefix == c.mac_prefix);
        REQUIRE(out.vendor_name == c.vendor_name);
        REQUIRE(out.is_private == c.is_private);
        REQUIRE(out.block_type == c.block_type);
        REQUIRE(out.last_update == c.last_update);

        results.clear();
    }
}

TEST_CASE("Vendor::operator<<") {
    struct test_case {
        Vendor      input;
        std::string expected;
    };

    const test_case cases[] = {
        {Vendor{"00:00:0C", "Cisco Systems, Inc", false, "MA-L", "2015/11/17"},
         "MAC prefix   00:00:0C\n"
         "Vendor name  Cisco Systems, Inc\n"
         "Private      no\n"
         "Block type   MA-L\n"
         "Last update  2015/11/17"},
        {Vendor{"00:48:54", "", true, "", "0001/01/01"},
         "MAC prefix   00:48:54\n"
         "Vendor name  -\n"
         "Private      yes\n"
         "Block type   -\n"
         "Last update  -"},
    };

    std::ostringstream oss;
    for (const auto& c : cases) {
        CAPTURE(c.input);

        oss << c.input;

        std::string out = oss.str();
        if (out != c.expected) {
            FAIL(std::format(
                "failed for '{}'\n\nexpected:\n{}\n\ngot:\n{}",
                c.input.mac_prefix,
                c.expected,
                out
            ));
        }

        oss.str("");
        oss.clear();
    }
}

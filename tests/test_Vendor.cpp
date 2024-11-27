#include <catch2/catch_test_macros.hpp>

#include <format>
#include <sstream>

#include "FinalAction.hpp"
#include "Vendor.hpp"
#include "cache.hpp"

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
            Vendor{"00:48:54", "", true, "", "0001/01/01"},
        }
    };

    for (auto& c : cases) {
        Vendor out = Vendor(c.input);

        REQUIRE(out.mac_prefix == c.expected.mac_prefix);
        REQUIRE(out.vendor_name == c.expected.vendor_name);
        REQUIRE(out.is_private == c.expected.is_private);
        REQUIRE(out.block_type == c.expected.block_type);
        REQUIRE(out.last_update == c.expected.last_update);
    }
}

// Ensures that a NULL text value returned from the database
// is correctly handled. Although the vendors table does not
// allow NULL text values, the check was implemented
// for protection against a compromised cache.
TEST_CASE("Vendor::Vendor(sqlite3_stmt* stmt)") {
    sqlite3_initialize();

    sqlite3* conn{};

    auto cleanup = finally([&] {
        sqlite3_close(conn);
        sqlite3_shutdown();
    });

    get_conn(conn, "testdata/poisoned.db");

    std::vector<Vendor> results = query_addr(conn, "00:00:0C");

    REQUIRE(!results.empty());
    REQUIRE(results.at(0).vendor_name == "");
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

#include <catch2/catch_test_macros.hpp>
#include <map>
#include <sstream>

#include "Vendor.hpp"
#include "cache/ConnR.hpp"
#include "cache/ConnRW.hpp"
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
            Vendor{0x00000C, "Cisco Systems, Inc", Registry::MA_L, "2015/11/17"}
        },
        test_case{
            // Non-quoted vendor name
            R"(00:00:0D,FIBRONICS LTD.,false,MA-L,2015/11/17)",
            Vendor{0x00000D, "FIBRONICS LTD.", Registry::MA_L, "2015/11/17"}
        },
        test_case{
            // Quoted vendor name, longer prefix
            R"(5C:F2:86:D,"BrightSky, LLC",false,MA-M,2019/07/02)",
            Vendor{0x5CF286D, "BrightSky, LLC", Registry::MA_M, "2019/07/02"}
        },
        test_case{
            // Non-quoted vendor name, longer prefix
            R"(8C:1F:64:F5:A,Telco Antennas Pty Ltd,false,MA-S,2021/10/13)",
            Vendor{0x8C1F64F5A, "Telco Antennas Pty Ltd", Registry::MA_S, "2021/10/13"}
        },
        test_case{
            // Escaped quotes inside quoted vendor name
            R"(2C:7A:FE,"IEE&E ""Black"" ops",false,MA-L,2010/07/26)",
            Vendor{0x2C7AFE, "IEE&E \"Black\" ops", Registry::MA_L, "2010/07/26"},
        },
        test_case{
            // Private block
            R"(00:48:54,,true,,0001/01/01)",
            Vendor{0x004854, "", Registry::Unknown, ""},
        },
        test_case{
            // The last update field is empty, but valid (comma present)
            R"(00:00:0D,FIBRONICS LTD.,false,MA-L,)",
            Vendor{0x00000D, "FIBRONICS LTD.", Registry::MA_L, ""},
        },
    };

    for (auto& c : cases) {
        CAPTURE(c.input);
        Vendor out = Vendor(c.input);

        REQUIRE(out.mac_prefix == c.expected.mac_prefix);
        REQUIRE(out.vendor_name == c.expected.vendor_name);
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
            CAPTURE(e);
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL("unexpected exception was thrown: '" + std::string{e.what()} + "'");
        }

        FAIL("no exception was thrown");
    }
}

// Checks for integer overflow during prefix retrieval.
//
// Also, ensures that a NULL text value returned from the database
// is correctly handled. Although the vendors table does not
// allow NULL text values, the check was implemented
// for protection against a compromised cache.
TEST_CASE("Vendor::Vendor(Stmt&)") {
    const std::string path = "file:memdb_vendor_stmt?mode=memory&cache=shared";

    ConnRW conn_rw{path, true};

    std::stringstream ss;
    ss << "Header\n8C:1F:64:FF:C,Invendis Technologies India Pvt Ltd,false,MA-S,2022/07/19";
    REQUIRE_NOTHROW(conn_rw.insert(ss));

    const int64_t expected = 0x8C1F64FFC;

    ConnR conn_r{path, true};

    std::vector<Vendor> results = conn_r.find_by_addr("8C:1F:64:FF:C");

    REQUIRE(!results.empty());
    REQUIRE(results.at(0).mac_prefix == expected);

    const ConnR conn_r2{"testdata/poisoned.db", true};

    results = conn_r2.find_by_addr("00:00:0C");

    REQUIRE(!results.empty());
    REQUIRE(results.at(0).vendor_name == "");
}

TEST_CASE("Vendor::operator<<") {
    struct test_case {
        Vendor      input;
        std::string expected;
    };

    const test_case cases[] = {
        {Vendor{0x00000C, "Cisco Systems, Inc", Registry::MA_L, "2015/11/17"},
         "MAC prefix   00:00:0C\n"
         "Vendor name  Cisco Systems, Inc\n"
         "Private      no\n"
         "Block type   MA-L\n"
         "Last update  2015/11/17"},
        {Vendor{0x004854, "", Registry::Unknown, "0001/01/01"},
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
        REQUIRE(out == c.expected);

        oss.str("");
        oss.clear();
    }
}

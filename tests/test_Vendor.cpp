#include <catch2/catch_test_macros.hpp>
#include <map>
#include <sstream>

#include "Vendor.hpp"
#include "cache/ConnR.hpp"
#include "exception.hpp"
#include "out.hpp"

// Tests whether the CSV lines are correctly parsed into a Vendor struct.
// The main challenge is that the file is comma-separated and sometimes
// vendor_name can contain a comma, therefore the CSV string needs to be
// wrapped in quotes. Vendor constructor must account for that when gathering
// values from line.
TEST_CASE("Vendor(string)") {
    struct test_case {
        std::string input;
        Vendor      expected;
    };

    const test_case cases[] = {
        test_case{
            // Quoted vendor name
            R"(00:00:0C,"Cisco Systems, Inc",false,MA-L,2015/11/17)",
            Vendor{0x00000C, "Cisco Systems, Inc", false, Registry::MA_L, "2015/11/17"}
        },
        test_case{
            // Non-quoted vendor name
            R"(00:00:0D,FIBRONICS LTD.,false,MA-L,2015/11/17)",
            Vendor{0x00000D, "FIBRONICS LTD.", false, Registry::MA_L, "2015/11/17"}
        },
        test_case{
            // Quoted vendor name, longer prefix
            R"(5C:F2:86:D,"BrightSky, LLC",false,MA-M,2019/07/02)",
            Vendor{0x5CF286D, "BrightSky, LLC", false, Registry::MA_M, "2019/07/02"}
        },
        test_case{
            // Non-quoted vendor name, longer prefix
            R"(8C:1F:64:F5:A,Telco Antennas Pty Ltd,false,MA-S,2021/10/13)",
            Vendor{0x8C1F64F5A, "Telco Antennas Pty Ltd", false, Registry::MA_S, "2021/10/13"}
        },
        test_case{
            // Escaped quotes inside quoted vendor name
            R"(2C:7A:FE,"IEE&E ""Black"" ops",false,MA-L,2010/07/26)",
            Vendor{0x2C7AFE, "IEE&E \"Black\" ops", false, Registry::MA_L, "2010/07/26"},
        },
        test_case{
            // Private block
            R"(00:48:54,,true,,0001/01/01)",
            Vendor{0x004854, "", true, Registry::Unknown, ""},
        },
        test_case{
            // Private block with non-empty name
            R"(00:48:54,non-empty,true,,0001/01/01)",
            Vendor{0x004854, "non-empty", true, Registry::Unknown, ""},
        },
        test_case{
            // The last update field is empty, but valid (comma present)
            R"(00:00:0D,FIBRONICS LTD.,false,MA-L,)",
            Vendor{0x00000D, "FIBRONICS LTD.", false, Registry::MA_L, ""},
        },
    };

    for (auto& c : cases) {
        CAPTURE(c.input);
        Vendor out = Vendor(c.input);

        REQUIRE(out == c.expected);
    }

    using namespace errors;

    const std::map<const std::string, const ParsingError> throw_cases = {
        {"", NoCommaError{""}},                                                             // Empty line
        {"00:00:00", NoCommaError{""}},                                                     // Only the prefix field
        {"00:00:00Vendor name", NoCommaError{""}},                                          // No commas
        {R"(5C:F2:86:D,"BrightSky, LLC,false,MA-M,2019/07/02)", QuotedTermSeqError{""}},    // No closing quote after vendor name
        {R"(5C:F2:86:D,"BrightSky, LLC"false,MA-M,2019/07/02)", QuotedTermSeqError{""}},    // Closing quote after vendor name present, but no comma
        {R"(00:00:00,"IEE&E ""Black"" ops,false,MA-L,2010/07/26)", QuotedTermSeqError{""}}, // No closing quote with escaped quote present
        {R"(00:00:0C,"Cisco Systems, Inc",no,MA-L,2015/11/17)", PrivateInvalidError{""}},   // Invalid private field
        {R"(00:00:0C,"Cisco Systems, Inc",falseMA-L,2015/11/17)", PrivateTermError{""}},    // No comma between private and block type fields
        {"00:00:00,", PrefixTermError{""}},                                                 // Comma after prefix ends the line
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

// Ensures that a NULL text value returned from the database
// is correctly handled. Although the vendors table does not
// allow NULL text values, the check was implemented
// for protection against a compromised cache.
TEST_CASE("NULL values") {
    const ConnR conn_r2{"testdata/poisoned.db", true};

    std::vector<Vendor> results = conn_r2.find_by_addr("00:00:0C");

    REQUIRE(!results.empty());
    REQUIRE(results.at(0).vendor_name == "");
}

TEST_CASE("operator<< out::csv") {
    struct test_case {
        Vendor      input;
        std::string expected;
    };

    const test_case cases[] = {
        {
            // Quoted vendor name
            Vendor{0x00000C, "Cisco Systems, Inc", false, Registry::MA_L, "2015/11/17"},
            R"(00:00:0C,"Cisco Systems, Inc",false,MA-L,2015/11/17)",
        },
        {
            // Non-quoted vendor name
            Vendor{0x00000D, "FIBRONICS LTD.", false, Registry::MA_L, "2015/11/17"},
            R"(00:00:0D,FIBRONICS LTD.,false,MA-L,2015/11/17)",
        },
        {
            // Quoted vendor name, longer prefix
            Vendor{0x5CF286D, "BrightSky, LLC", false, Registry::MA_M, "2019/07/02"},
            R"(5C:F2:86:D,"BrightSky, LLC",false,MA-M,2019/07/02)",
        },
        {
            // Non-quoted vendor name, longer prefix
            Vendor{0x8C1F64F5A, "Telco Antennas Pty Ltd", false, Registry::MA_S, "2021/10/13"},
            R"(8C:1F:64:F5:A,Telco Antennas Pty Ltd,false,MA-S,2021/10/13)",
        },
        {
            // Escaped quotes inside quoted vendor name
            Vendor{0x2C7AFE, "IEE&E \"Black\" ops", false, Registry::MA_L, "2010/07/26"},
            R"(2C:7A:FE,"IEE&E ""Black"" ops",false,MA-L,2010/07/26)",
        },
        {
            // Private block
            Vendor{0x004854, "", true, Registry::Unknown, ""},
            R"(00:48:54,,true,,)",
        },
    };

    std::ostringstream oss;
    for (const auto& c : cases) {

        oss << out::csv << c.input;

        std::string out = oss.str();
        REQUIRE(out == c.expected);

        oss.str("");
        oss.clear();
    }
}

TEST_CASE("operator<< out::json") {
    struct test_case {
        Vendor      input;
        std::string expected;
    };

    const test_case cases[] = {
        {
            // Quoted vendor name
            Vendor{0x00000C, "Cisco Systems, Inc", false, Registry::MA_L, "2015/11/17"},
            R"({"macPrefix":"00:00:0C","vendorName":"Cisco Systems, Inc","private":false,"blockType":"MA-L","lastUpdate":"2015/11/17"})",
        },
        {
            // Non-quoted vendor name
            Vendor{0x00000D, "FIBRONICS LTD.", false, Registry::MA_L, "2015/11/17"},
            R"({"macPrefix":"00:00:0D","vendorName":"FIBRONICS LTD.","private":false,"blockType":"MA-L","lastUpdate":"2015/11/17"})",
        },
        {
            // Quoted vendor name, longer prefix
            Vendor{0x5CF286D, "BrightSky, LLC", false, Registry::MA_M, "2019/07/02"},
            R"({"macPrefix":"5C:F2:86:D","vendorName":"BrightSky, LLC","private":false,"blockType":"MA-M","lastUpdate":"2019/07/02"})",
        },
        {
            // Non-quoted vendor name, longer prefix
            Vendor{0x8C1F64F5A, "Telco Antennas Pty Ltd", false, Registry::MA_S, "2021/10/13"},
            R"({"macPrefix":"8C:1F:64:F5:A","vendorName":"Telco Antennas Pty Ltd","private":false,"blockType":"MA-S","lastUpdate":"2021/10/13"})",
        },
        {
            // Escaped quotes inside quoted vendor name, ampersand
            Vendor{0x2C7AFE, "IEE&E \"Black\" ops", false, Registry::MA_L, "2010/07/26"},
            R"({"macPrefix":"2C:7A:FE","vendorName":"IEE\u0026E \"Black\" ops","private":false,"blockType":"MA-L","lastUpdate":"2010/07/26"})",
        },
        {
            // Unquoted vendor name, ampersand
            Vendor{0x0060D3, "AT&T", false, Registry::MA_L, "2016/10/12"},
            R"({"macPrefix":"00:60:D3","vendorName":"AT\u0026T","private":false,"blockType":"MA-L","lastUpdate":"2016/10/12"})",
        },
        {
            // Private block
            Vendor{0x004854, "", true, Registry::Unknown, ""},
            R"({"macPrefix":"00:48:54","vendorName":"","private":true,"blockType":"","lastUpdate":""})",
        },
    };

    std::ostringstream oss;
    for (const auto& c : cases) {

        oss << out::json << c.input;

        std::string out = oss.str();
        REQUIRE(out == c.expected);

        oss.str("");
        oss.clear();
    }
}

TEST_CASE("operator<< out::regular") {
    struct test_case {
        Vendor      input;
        std::string expected;
    };

    const test_case cases[] = {
        {Vendor{0x00000C, "Cisco Systems, Inc", false, Registry::MA_L, "2015/11/17"},
         "MAC prefix   00:00:0C\n"
         "Vendor name  Cisco Systems, Inc\n"
         "Private      no\n"
         "Block type   MA-L\n"
         "Last update  2015/11/17"},
        {Vendor{0x004854, "", true, Registry::Unknown, ""},
         "MAC prefix   00:48:54\n"
         "Vendor name  -\n"
         "Private      yes\n"
         "Block type   -\n"
         "Last update  -"},
        {Vendor{0x004854, "non-empty", true, Registry::Unknown, ""},
         "MAC prefix   00:48:54\n"
         "Vendor name  non-empty\n"
         "Private      yes\n"
         "Block type   -\n"
         "Last update  -"},
    };

    std::ostringstream oss;
    for (const auto& c : cases) {

        oss << out::regular << c.input;

        std::string out = oss.str();
        REQUIRE(out == c.expected);

        oss.str("");
        oss.clear();
    }
}

TEST_CASE("operator<< out::xml") {
    struct test_case {
        Vendor      input;
        std::string expected;
    };

    const test_case cases[] = {
        {
            // Quoted vendor name
            Vendor{0x00000C, "Cisco Systems, Inc", false, Registry::MA_L, "2015/11/17"},
            R"(<VendorMapping mac_prefix="00:00:0C" vendor_name="Cisco Systems, Inc"></VendorMapping>)",
        },
        {
            // Non-quoted vendor name
            Vendor{0x00000D, "FIBRONICS LTD.", false, Registry::MA_L, "2015/11/17"},
            R"(<VendorMapping mac_prefix="00:00:0D" vendor_name="FIBRONICS LTD."></VendorMapping>)",
        },
        {
            // Quoted vendor name, longer prefix
            Vendor{0x5CF286D, "BrightSky, LLC", false, Registry::MA_M, "2019/07/02"},
            R"(<VendorMapping mac_prefix="5C:F2:86:D" vendor_name="BrightSky, LLC"></VendorMapping>)",
        },
        {
            // Non-quoted vendor name, longer prefix
            Vendor{0x8C1F64F5A, "Telco Antennas Pty Ltd", false, Registry::MA_S, "2021/10/13"},
            R"(<VendorMapping mac_prefix="8C:1F:64:F5:A" vendor_name="Telco Antennas Pty Ltd"></VendorMapping>)",
        },
        {
            // Escaped quotes inside quoted vendor name, ampersand
            Vendor{0x2C7AFE, "IEE&E \"Black\" ops", false, Registry::MA_L, "2010/07/26"},
            R"(<VendorMapping mac_prefix="2C:7A:FE" vendor_name="IEE&amp;E &#34;Black&#34; ops"></VendorMapping>)",
        },
        {
            // Unquoted vendor name, ampersand
            Vendor{0x0060D3, "AT&T", false, Registry::MA_L, "2016/10/12"},
            R"(<VendorMapping mac_prefix="00:60:D3" vendor_name="AT&amp;T"></VendorMapping>)",
        },
        {
            // Private block
            Vendor{0x004854, "", true, Registry::Unknown, ""},
            R"(<VendorMapping mac_prefix="00:48:54" vendor_name=""></VendorMapping>)",
        },
    };

    std::ostringstream oss;
    for (const auto& c : cases) {

        oss << out::xml << c.input;

        std::string out = oss.str();
        REQUIRE(out == c.expected);

        oss.str("");
        oss.clear();
    }
}

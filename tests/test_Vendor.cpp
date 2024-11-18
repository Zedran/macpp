#include <catch2/catch_test_macros.hpp>

#include "Vendor.hpp"

// Tests whether the CSV lines are correctly parsed into a Vendor struct.
// The main challenge is that the file is comma-separated and sometimes
// vendor_name can contain a comma, therefore the CSV string needs to be
// wrapped in quotes. Vendor constructor must account for that when gathering
// values from line.
TEST_CASE("Vendor") {
    struct test_case {
        std::string input;
        Vendor      expected;
    };

    test_case cases[] = {
        test_case{
            R"(00:00:0C,"Cisco Systems, Inc",false,MA-L,2015/11/17)",
            Vendor{"00:00:0C", "Cisco Systems, Inc", false, "MA-L", "2015/11/17"}
        },
        test_case{
            R"(00:00:0D,FIBRONICS LTD.,false,MA-L,2015/11/17)",
            Vendor{"00:00:0D", "FIBRONICS LTD.", false, "MA-L", "2015/11/17"}
        },
        test_case{
            R"(5C:F2:86:D,"BrightSky, LLC",false,MA-M,2019/07/02)",
            Vendor{"5C:F2:86:D", "BrightSky, LLC", false, "MA-M", "2019/07/02"}
        },
        test_case{
            R"(8C:1F:64:F5:A,Telco Antennas Pty Ltd,false,MA-S,2021/10/13)",
            Vendor{"8C:1F:64:F5:A", "Telco Antennas Pty Ltd", false, "MA-S", "2021/10/13"}
        },
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

#include <catch2/catch_test_macros.hpp>
#include <map>

#include "exception.hpp"
#include "utils.hpp"

// Ensures that the query statement returned from build_query_by_id_stmt
// contains the correct number of placeholders.
TEST_CASE("build_query_by_id_stmt") {
    struct test_case {
        size_t      length;
        std::string expected;
    };

    const test_case cases[] = {
        test_case{1, "SELECT * FROM vendors WHERE id IN (?)"},
        test_case{2, "SELECT * FROM vendors WHERE id IN (?,?)"},
        test_case{3, "SELECT * FROM vendors WHERE id IN (?,?,?)"},
    };

    for (auto& c : cases) {
        CAPTURE(c.length);

        std::string out = build_query_by_id_stmt(c.length);
        REQUIRE(out == c.expected);
    }
}

// Ensures that construct_queries generates correct number of prefixes
// in integer form and throws an error when no prefix of valid length
// can be constructed (query string too short).
TEST_CASE("construct_queries") {
    struct test_case {
        std::string          input;
        std::vector<int64_t> expected;
    };

    const test_case cases[] = {
        test_case{"000000", {0x000000}},
        test_case{"5CF286D", {0x5CF286, 0x5CF286D}},
        test_case{"8C1F64F5A", {0x8C1F64, 0x8C1F64F, 0x8C1F64F5A}},
        test_case{"8C1F64F5A000", {0x8C1F64, 0x8C1F64F, 0x8C1F64F5A}},
        test_case{"000222", {0x000222}},
    };

    for (const auto& c : cases) {
        std::vector<int64_t> out = construct_queries(c.input);

        CAPTURE(c.input);
        REQUIRE(out == c.expected);
    }

    const auto too_short = errors::Error{"specified MAC address is too short"};
    const auto invalid   = errors::Error{"specified MAC address contains invalid characters"};

    const std::map<const std::string, const errors::Error&> throw_cases = {
        {"", too_short},
        {"00", too_short},
        {"00:00", too_short},
        {"0002w22", invalid},
        {"00000w", invalid},
        {"xxxxxx", invalid}
    };

    for (const auto& [input, expected_error] : throw_cases) {
        CAPTURE(input);

        try {
            construct_queries(input);
        } catch (const errors::Error& e) {
            CAPTURE(e);
            REQUIRE(strcmp(e.what(), expected_error.what()) == 0);
            continue;
        } catch (const std::exception& e) {
            FAIL("unexpected exception was thrown: '" + std::string{e.what()} + "'");
        }

        FAIL("no exception was thrown");
    }
}

TEST_CASE("suppress_like_wildcards") {
    const std::map<std::string, std::string> cases = {
        {"DIG_LINK", R"(DIG\_LINK)"},
        {"Ad%ced Micro Devices", R"(Ad\%ced Micro Devices)"},
        {"Cisco Systems, Inc", "Cisco Systems, Inc"},
        {"_", R"(\_)"},
    };

    for (const auto& [input, expected] : cases) {
        std::string out = suppress_like_wildcards(input);

        CAPTURE(input);
        REQUIRE(out == expected);
    }
}

TEST_CASE("get_ieee_block") {
    struct test_case {
        size_t      block_length;
        std::string input;
        std::string expected;
    };

    const test_case cases[] = {
        test_case{6, "000000", "000000"},
        test_case{6, "5CF286D", "5CF286"},
        test_case{7, "5CF286D", "5CF286D"},
        test_case{6, "8C1F64F5A", "8C1F64"},
        test_case{9, "8C1F64F5A", "8C1F64F5A"},
        test_case{9, "8C1F64F5A000", "8C1F64F5A"},
    };

    std::optional<std::string> out;

    for (const auto& c : cases) {
        CAPTURE(c.input, c.block_length);

        out = get_ieee_block(c.input, c.block_length);

        REQUIRE(out.has_value());
        REQUIRE(out.value() == c.expected);
    }

    const test_case nullopt_cases[] = {
        {6, "00", ""},
        {7, "00", ""},
        {9, "00", ""},
    };

    for (const auto& nc : nullopt_cases) {
        CAPTURE(nc.input, nc.block_length);

        out = get_ieee_block(nc.input, nc.block_length);
        REQUIRE(!out.has_value());
    }
}

// Ensures that prefix_to_id returns a correct numerical value.
TEST_CASE("prefix_to_id") {
    const std::map<std::string, int64_t> cases = {
        {"000000", 0x000000},
        {"101010", 0x101010},
        {"FFFFFFFFF", 0xFFFFFFFFF},
    };

    for (auto& [input, expected] : cases) {
        REQUIRE(prefix_to_id(input) == expected);
    }
}

TEST_CASE("replace_escaped_quotes") {
    const std::map<const std::string, const std::string> cases = {
        {R"(IEE&E ""Black"" ops)", R"(IEE&E "Black" ops)"},
        {R"(")", R"(")"},
        {R"("")", R"(")"},
        {R"(abc "def" ghe)", R"(abc "def" ghe)"},
        {R"(abc)", R"(abc)"},
    };

    for (auto& [input, expected] : cases) {
        std::string mod = input;

        replace_escaped_quotes(mod);
        REQUIRE(mod == expected);
    }
}

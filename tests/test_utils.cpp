#include <catch2/catch_test_macros.hpp>
#include <map>
#include <sstream>

#include "internal/internal.hpp"
#include "utils.hpp"

// Ensures that the query statement returned from build_query_by_id_stmt
// contains the correct number of placeholders.
TEST_CASE("build_query_by_id_stmt") {
    struct test_case {
        size_t      length;
        std::string expected;
    };

    const test_case cases[] = {
        test_case{1, "SELECT * FROM vendors WHERE id IN (?);"},
        test_case{2, "SELECT * FROM vendors WHERE id IN (?,?);"},
        test_case{3, "SELECT * FROM vendors WHERE id IN (?,?,?);"},
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
        std::string               input;
        std::vector<std::int64_t> expected;
    };

    const test_case cases[] = {
        test_case{"000000", {0}},
        test_case{"000000", {0}},
        test_case{"5CF286D", {6091398, 97462381}},
        test_case{"8C1F64F5A", {9183076, 146929231, 37613883226}},
        test_case{"8C1F64F5A000", {9183076, 146929231, 37613883226}},
        test_case{"000222", {546}},
    };

    for (auto& c : cases) {
        std::vector<int64_t> out = construct_queries(c.input);

        if (out != c.expected) {
            std::ostringstream oss;
            oss << "failed for case '" << c.input << "': got '";

            internal::print_vector(oss, out);
            oss << "', expected '";

            internal::print_vector(oss, c.expected);
            oss << "'";

            FAIL(oss.str());
        }
    }

    const std::string bad_cases[] = {
        "",
        "00",
        "00:00",
        "0002w22",
        "xxxxxx",
        "00000w",
    };

    for (auto& c : bad_cases) {
        REQUIRE_THROWS_AS(construct_queries(c), std::exception);
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

        if (out != expected) {
            FAIL_CHECK(std::format("failed for '{}': got '{}'", input, out));
        }
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

    for (auto& c : cases) {
        CAPTURE(c.input, c.block_length);
        REQUIRE(get_ieee_block(c.input, c.block_length) == c.expected);
    }
}

// Ensures that prefix_to_id returns a correct numerical value.
TEST_CASE("prefix_to_id") {
    const std::map<std::string, int64_t> cases = {
        {"000000", 0},
        {"101010", 1052688},
        {"101010", 1052688},
        {"FFFFFFFFF", 68719476735},
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

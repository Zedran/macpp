#include <catch2/catch_test_macros.hpp>
#include <map>
#include <sstream>

#include "AppError.hpp"
#include "utils.hpp"

// A helper function that directs a space-separated list of vec elements to oss.
void append_vector(std::ostringstream& oss, const std::vector<int64_t>& vec) {
    for (size_t i = 0; i < vec.size(); i++)
        oss << vec[i] << (i != vec.size() - 1 ? " " : "");
}

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
        test_case{"00:00:00", {0}},
        test_case{"000000", {0}},
        test_case{"5C:F2:86:D", {6091398, 97462381}},
        test_case{"8C:1F:64:F5:A", {9183076, 146929231, 37613883226}},
        test_case{"8C:1F:64:F5:A0:00", {9183076, 146929231, 37613883226}},
    };

    for (auto& c : cases) {
        std::vector<int64_t> out = construct_queries(c.input);

        if (out != c.expected) {
            std::ostringstream oss;
            oss << "failed for case '" << c.input << "': got '";

            append_vector(oss, out);
            oss << "', expected '";

            append_vector(oss, c.expected);
            oss << "'";

            FAIL(oss.str());
        }
    }

    const std::string bad_cases[] = {
        "",
        "00",
        "00:00",
        "0::::::0", // Extra separators allowed, but should not count as digits.
    };

    for (auto& c : bad_cases) {
        REQUIRE_THROWS_AS(construct_queries(c), AppError);
    }
}

TEST_CASE("get_ieee_block") {
    struct test_case {
        size_t      block_length;
        std::string input;
        std::string expected;
    };

    const test_case cases[] = {
        test_case{6, "00:00:00", "000000"},
        test_case{6, "000000", "000000"},
        test_case{6, "5C:F2:86:D", "5CF286"},
        test_case{7, "5C:F2:86:D", "5CF286D"},
        test_case{6, "8C:1F:64:F5:A", "8C1F64"},
        test_case{9, "8C:1F:64:F5:A", "8C1F64F5A"},
        test_case{9, "8C:1F:64:F5:A0:00", "8C1F64F5A"},
    };

    for (auto& c : cases) {
        REQUIRE(get_ieee_block(c.input, c.block_length) == c.expected);
    }
}

// Ensures that prefix_to_id returns a correct numerical value,
// whether the specified string contains delimiters or not.
TEST_CASE("prefix_to_id") {
    const std::map<std::string, int64_t> cases = {
        {"00:00:00", 0},
        {"10:10:10", 1052688},
        {"101010", 1052688},
        {"FF:FF:FF:FF:F", 68719476735},
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

#include <catch2/catch_test_macros.hpp>
#include <map>

#include "utils.hpp"

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

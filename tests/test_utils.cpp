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

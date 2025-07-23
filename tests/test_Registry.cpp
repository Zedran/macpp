#include <catch2/catch_test_macros.hpp>

#include <map>

#include "Registry.hpp"

TEST_CASE("from_registry") {
    using enum Registry;

    const std::map<Registry, std::string> cases = {
        {CID, "CID"},
        {IAB, "IAB"},
        {MA_L, "MA-L"},
        {MA_M, "MA-M"},
        {MA_S, "MA-S"},
        {Unknown, "-"},
    };

    for (const auto& [input, expected] : cases) {
        CAPTURE(input);
        REQUIRE(from_registry(input) == expected);
    }
}

TEST_CASE("to_registry") {
    using enum Registry;

    const std::map<std::string, Registry> cases = {
        {"CID", CID},
        {"IAB", IAB},
        {"MA-L", MA_L},
        {"MA-M", MA_M},
        {"MA-M", MA_S},
        {"", Unknown},
        {"abc", Unknown},
        {"-", Unknown},
    };

    for (const auto& [input, expected] : cases) {
        CAPTURE(input);
        REQUIRE(to_registry(input) == expected);
    }
}

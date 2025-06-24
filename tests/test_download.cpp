#include <catch2/catch_test_macros.hpp>
#include <format>

#include "download.hpp"
#include "exception.hpp"

TEST_CASE("get_local_file") {
    REQUIRE_NOTHROW(get_local_file("testdata/update.csv"));

    const std::string bad_path = "testdata/non-existent.csv";

    const auto expected_error = errors::Error{"file '" + bad_path + "' not found"};

    try {
        get_local_file(bad_path);
        FAIL("no exception was thrown");
    } catch (const errors::Error& e) {
        REQUIRE((e.what() == expected_error.what()) == 0);
    } catch (const std::exception& e) {
        FAIL(std::format("unexpected exception was thrown: '{}'", e.what()));
    }
}

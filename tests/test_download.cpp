#include <catch2/catch_test_macros.hpp>
#include <format>

#include "download.hpp"
#include "exception.hpp"

TEST_CASE("get_local_file") {
    REQUIRE_NOTHROW(get_local_file("testdata/update.csv"));

    try {
        get_local_file("testdata/non-existent.csv");
        FAIL("no exception was thrown");
    } catch (const errors::Error& e) {
        REQUIRE(e == errors::UpdatePathError);
    } catch (const std::exception& e) {
        FAIL(std::format("unexpected exception was thrown: '{}'", e.what()));
    }
}

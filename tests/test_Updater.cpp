#include <catch2/catch_test_macros.hpp>
#include <filesystem>

#include "exception.hpp"
#include "update/Reader.hpp"

TEST_CASE("Reader") {
    REQUIRE_NOTHROW(Reader{"testdata/update.csv"});

    const std::string bad_path = "testdata/non-existent.csv";

    const auto expected_error = errors::Error{"file '" + bad_path + "' not found"};

    try {
        Reader r{bad_path};
        FAIL("no exception was thrown");
    } catch (const errors::Error& e) {
        CAPTURE(e);
        REQUIRE((e.what() == expected_error.what()) == 0);
    } catch (const std::exception& e) {
        FAIL("unexpected exception was thrown: " + std::string{e.what()} + "'");
    }

    namespace fs = std::filesystem;

    const std::string large_path = "testdata/large.csv";

    if (!fs::exists(large_path) || fs::file_size(large_path) < Updater::MAX_FSIZE) {
        std::ofstream large_file{large_path, std::ios::binary};

        large_file.seekp(Updater::MAX_FSIZE);
        const char zero = 0;
        large_file.write(&zero, 1);
    }

    REQUIRE_THROWS_AS(Reader{large_path}, errors::NetworkError);
}

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <filesystem>

#include "exception.hpp"
#include "update/Reader.hpp"

TEST_CASE("Reader") {
    REQUIRE_NOTHROW(Reader{"testdata/update.csv"});

    const std::string bad_path = "testdata/non-existent.csv";

    const auto expected_error = errors::Error{"file '" + bad_path + "' not found"};

    REQUIRE_THROWS_MATCHES(
        Reader{bad_path},
        errors::Error,
        Catch::Matchers::Message(expected_error.what())
    );

    namespace fs = std::filesystem;

    const std::string large_path = "testdata/large.csv";

    if (!fs::exists(large_path) || fs::file_size(large_path) < Updater::MAX_FSIZE) {
        std::ofstream large_file{large_path, std::ios::binary};

        large_file.seekp(Updater::MAX_FSIZE);
        const char zero = 0;
        large_file.write(&zero, 1);
    }

    REQUIRE_THROWS_AS(Reader{large_path}, errors::UpdateError);
}

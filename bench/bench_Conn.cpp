#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <vector>

#include "cache/ConnR.hpp"
#include "utils.hpp"

TEST_CASE("ConnR::find_by_addr") {
    // Each entry requires more placeholders than the previous one.
    const std::array<std::string, 4> test_values = {
        "000000",       // 1
        "741AE0C",      // 2
        "0050C2003",    // 3
        "024201234567", // 4
    };

    for (size_t i = 0; i < test_values.size(); i++) {
        REQUIRE(construct_queries(test_values[i]).size() == i + 1);
    }

    constexpr size_t T = 1000;

    std::vector<std::string> addresses{};
    addresses.reserve(T);

    for (size_t i = 0; i < T / test_values.size(); i++) {
        addresses.push_back(test_values[0]);
        addresses.push_back(test_values[1]);
        addresses.push_back(test_values[2]);
        addresses.push_back(test_values[3]);
    }

    REQUIRE(addresses.size() == T);

    ConnR conn{"testdata/sample.db", true};

    BENCHMARK("1 term") {
        return conn.find_by_addr({addresses.data(), 1});
    };

    BENCHMARK("4 terms") {
        return conn.find_by_addr({addresses.data(), 4});
    };

    BENCHMARK("10 terms") {
        return conn.find_by_addr({addresses.data(), 10});
    };

    BENCHMARK("52 terms") {
        return conn.find_by_addr({addresses.data(), 52});
    };

    BENCHMARK("100 terms") {
        return conn.find_by_addr({addresses.data(), 100});
    };

    BENCHMARK("1000 terms") {
        return conn.find_by_addr(addresses);
    };
}

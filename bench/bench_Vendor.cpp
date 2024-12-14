#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

#include "Vendor.hpp"

TEST_CASE("Benchmark Vendor::Vendor(std::string&)") {
    BENCHMARK("quoted vendor name") {
        return Vendor(R"(00:00:0C,"Cisco Systems, Inc",false,MA-L,2015/11/17)");
    };
    BENCHMARK("unquoted vendor name") {
        return Vendor(R"(00:00:0D,FIBRONICS LTD.,false,MA-L,2015/11/17)");
    };
}

#include <catch2/catch_test_macros.hpp>

#include "cache/ConnRW.hpp"
#include "cache/StmtPool.hpp"

TEST_CASE("StmtPool::get") {
    ConnRW conn{"testdata/sample.db", true};

    StmtPool pool;

    for (size_t i = 0; i < 4; i++) {
        REQUIRE_NOTHROW(pool.get(conn.get(), i + 1));
    }
}

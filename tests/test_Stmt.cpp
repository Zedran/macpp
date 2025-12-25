#include <catch2/catch_test_macros.hpp>

#include "cache/ConnRW.hpp"
#include "cache/Stmt.hpp"

TEST_CASE("Stmt::Stmt: errors") {
    const ConnRW conn{"file:memdb_stmt-errors?mode=memory&cache=shared", true};
    REQUIRE_THROWS(Stmt{conn.get(), "SELEC * FROM vendors"});
}

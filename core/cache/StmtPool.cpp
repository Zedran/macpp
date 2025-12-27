#include <cassert>

#include "cache/StmtPool.hpp"
#include "utils.hpp"

StmtPool::StmtPool() noexcept {}

Stmt& StmtPool::get(sqlite3* const conn, const size_t num_queries) noexcept {
    assert(num_queries > 0);
    assert(num_queries <= stmts.size());

    const size_t index = num_queries - 1;

    if (!stmts[index]) {
        stmts[index].emplace(conn, build_find_by_addr_stmt(num_queries));
    }

    return *stmts[index];
}

#pragma once

#include <array>
#include <optional>
#include <sqlite3.h>

#include "Stmt.hpp"

// Helper class for Conn::find_by_addr. It caches query statements
// with different number of placeholders to allow their reuse
// during multi-term search.
class StmtPool {
    std::array<std::optional<Stmt>, 4> stmts;

public:
    StmtPool() noexcept = default;

    StmtPool(const StmtPool& other)            = delete;
    StmtPool& operator=(const StmtPool& other) = delete;

    Stmt& get(sqlite3* const conn, const size_t num_queries) noexcept;
};

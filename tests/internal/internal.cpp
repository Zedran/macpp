#include "internal.hpp"

namespace internal {

int open_mem_db(sqlite3*& conn) {
    constexpr const char* create_table_stmt =
        R"(CREATE TABLE vendors (
            id      INTEGER PRIMARY KEY,
            addr    TEXT NOT NULL,
            name    TEXT NOT NULL,
            private BOOLEAN,
            block   TEXT NOT NULL,
            updated TEXT NOT NULL
        );
    )";

    int code = -1;

    if ((code = sqlite3_open(":memory:", &conn)) != SQLITE_OK)
        return code;

    code = sqlite3_exec(conn, create_table_stmt, nullptr, nullptr, nullptr);

    return code;
}

void print_vector(std::ostringstream& oss, const std::vector<int64_t>& vec) {
    for (size_t i = 0; i < vec.size(); i++)
        oss << vec[i] << (i != vec.size() - 1 ? " " : "");
}

} // namespace internal

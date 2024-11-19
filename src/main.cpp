#include <curl/curl.h>
#include <iostream>

#include "AppError.hpp"
#include "cache.hpp"
#include "utils.hpp"

int main() {
    sqlite3* conn{};

    auto close = finally([&] {
        if (sqlite3_close(conn) != SQLITE_OK) {
            std::cerr << "failed to close connection" << std::endl;
        }
    });

    try {
        get_conn(conn);
    } catch (const AppError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }

    return 0;
}

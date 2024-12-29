#include <catch2/catch_test_macros.hpp>
#include <curl/curl.h>
#include <sqlite3.h>

#include "FinalAction.hpp"
#include "exception.hpp"
#include "internal/internal.hpp"

TEST_CASE("CacheError") {
    sqlite3_initialize();

    sqlite3* conn{};
    internal::open_mem_db(conn);

    const auto cleanup = finally([&] {
        sqlite3_close(conn);
        sqlite3_shutdown();
    });

    const errors::CacheError BASE("sqlite3 error");

    sqlite3_exec(conn, "A", nullptr, nullptr, nullptr);
    errors::CacheError out = BASE.wrap(conn);
    errors::CacheError expected("sqlite3 error: (1) near \"A\": syntax error");
    CAPTURE(out);
    REQUIRE(out == expected);

    CAPTURE("nullptr conn");
    out      = BASE.wrap(static_cast<sqlite3*>(nullptr));
    expected = BASE;
    REQUIRE(out == expected);

    CAPTURE("message");
    out      = BASE.wrap("boom");
    expected = errors::CacheError("sqlite3 error: boom");
    REQUIRE(out == expected);

    CAPTURE("nullptr message");
    out      = BASE.wrap(static_cast<const char*>(nullptr));
    expected = BASE;
    REQUIRE(out == expected);
}

TEST_CASE("NetworkError") {
    errors::NetworkError out = errors::NetworkError("download error").wrap(static_cast<CURLcode>(1));
    errors::NetworkError exp = errors::NetworkError("download error: Unsupported protocol");
    CAPTURE(out);
    REQUIRE(out == exp);
}

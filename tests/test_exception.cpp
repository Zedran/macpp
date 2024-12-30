#include <catch2/catch_test_macros.hpp>
#include <curl/curl.h>
#include <sqlite3.h>

#include "FinalAction.hpp"
#include "exception.hpp"
#include "internal/internal.hpp"

TEST_CASE("Error") {
    errors::CacheError BASE("sqlite3 error");
    errors::CacheError out = BASE.wrap("boom");

    // other must be of different length than BASE and out
    errors::CacheError other("incredibly mysterious error");
    REQUIRE(strlen(other.what()) != strlen(out.what()));
    REQUIRE(strlen(other.what()) != strlen(BASE.what()));

    REQUIRE(BASE == BASE);
    REQUIRE(out == out);
    REQUIRE(other == other);
    REQUIRE(BASE == out);
    REQUIRE(out == BASE);
    REQUIRE(!(BASE == other));
    REQUIRE(!(other == BASE));
    REQUIRE(!(out == other));
    REQUIRE(!(other == out));

    REQUIRE(BASE != other);
    REQUIRE(other != BASE);
    REQUIRE(out != other);
    REQUIRE(other != out);
    REQUIRE(!(BASE != BASE));
    REQUIRE(!(out != out));
    REQUIRE(!(BASE != out));
    REQUIRE(!(out != BASE));

    REQUIRE(BASE.is_exactly(BASE));
    REQUIRE(out.is_exactly(out));
    REQUIRE(other.is_exactly(other));
    REQUIRE(!BASE.is_exactly(out));
    REQUIRE(!out.is_exactly(BASE));
    REQUIRE(!BASE.is_exactly(other));
    REQUIRE(!out.is_exactly(other));
}

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
    errors::NetworkError out = errors::NetworkError("network error").wrap(static_cast<CURLcode>(1));
    errors::NetworkError exp = errors::NetworkError("network error: Unsupported protocol");
    CAPTURE(out);
    REQUIRE(out == exp);
}

TEST_CASE("ParsingError") {
    errors::ParsingError out = errors::ParsingError("parsing error").wrap("boom");
    errors::ParsingError exp = errors::ParsingError("parsing error: in line 'boom'");
    CAPTURE(out);
    REQUIRE(out == exp);
}

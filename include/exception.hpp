#pragma once

#include <curl/curl.h>
#include <format>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace errors {

class Error : public std::runtime_error {
    using std::runtime_error::runtime_error;

public:
    // Returns true if the object's message equals exactly other's message
    // (includes wrapped info in comparison).
    bool is_exactly(const Error& other) const noexcept {
        return std::string(this->what()) == std::string(other.what());
    }

    // Returns true if at least base message of the object and other
    // are equal (disregards wrapped info during comparison).
    bool operator==(const Error& other) const noexcept {
        std::string first  = this->what();
        std::string second = other.what();

        if (first == second) {
            return true;
        }

        if (first.length() > second.length()) {
            return first.starts_with(second);
        }
        return second.starts_with(first);
    }

    // Returns true if the object and other do not share even the base message
    // (disregards wrapped info during comparison).
    bool operator!=(const Error& other) const noexcept {
        std::string first  = this->what();
        std::string second = other.what();

        if (first == second) {
            return false;
        }

        if (first.length() > second.length()) {
            return !(first.starts_with(second));
        }
        return !(second.starts_with(first));
    }

    friend std::ostream& operator<<(std::ostream& os, const Error& e) {
        return os << e.what();
    }
};

class CacheError : public Error {
public:
    explicit CacheError(const std::string& msg) : Error{msg} {}

    // Returns a new CacheError that wraps sqlite3 error information retrieved
    // from sqlite3 object.
    explicit CacheError(const std::string& msg, sqlite3* const conn)
        : Error{std::format("{}: ({}), {}", msg, sqlite3_errcode(conn), sqlite3_errmsg(conn))} {}

    // Returns a new CacheError that wraps sqlite3 error information retrieved
    // from SQLite response code.
    explicit CacheError(const std::string& msg, const int code)
        : Error{std::format("{}: ({}) {}", msg, code, sqlite3_errstr(code))} {}
};

class NetworkError : public Error {
public:
    explicit NetworkError(const std::string& msg) : Error{msg} {}

    // Constructs a new NetworkError that contains an error message retrieved
    // from CURL.
    explicit NetworkError(const std::string& msg, const CURLcode code)
        : Error{std::format("{}: ({}) {}", msg, static_cast<int>(code), curl_easy_strerror(code))} {}
};

class ParsingError : public Error {
    std::string line;

protected:
    explicit ParsingError(const std::string& msg, const std::string& line) : Error{msg}, line{line} {}

public:
    friend std::ostream& operator<<(std::ostream& os, const ParsingError& e) {
        return os << e.what() << ": '" << e.line << '\'';
    }
};

class NoCommaError : public ParsingError {
public:
    explicit NoCommaError(const std::string& line = "") : ParsingError{"no comma found in CSV line", line} {}
};

// Thrown if quoted vendor name field is not terminated with '",' sequence,
// e.g. ',"Cisco Systems, Inc,false'
class QuotedTermSeqError : public ParsingError {
public:
    explicit QuotedTermSeqError(const std::string& line = "") : ParsingError{"closing '\",' sequence for vendor name not found", line} {}
};

// Thrown if unquoted vendor name field does not end with a comma
class UnquotedTermError : public ParsingError {
public:
    explicit UnquotedTermError(const std::string& line = "") : ParsingError{"no comma after unquoted vendor name", line} {}
};

class PrivateInvalidError : public ParsingError {
public:
    explicit PrivateInvalidError(const std::string& line = "") : ParsingError{"invalid value of private field", line} {}
};

class PrivateTermError : public ParsingError {
public:
    explicit PrivateTermError(const std::string& line = "") : ParsingError{"no comma between private and block type fields", line} {}
};

class BlockTypeTermError : public ParsingError {
public:
    explicit BlockTypeTermError(const std::string& line = "") : ParsingError{"no comma between block type and last update fields", line} {}
};

const Error AddrInvalidError("specified MAC address contains invalid characters");

const Error AddrTooShortError("specified MAC address is too short");

// Thrown if --addr option is given with no value
const Error EmptyAddrError("empty MAC address");

// Thrown if --name option is given with no value
const Error EmptyNameError("empty vendor name");

// Thrown if no command is given (--addr, --name, --update).
const Error NoActionError("no action specified");

const CacheError CachePathError("could not resolve cache path");

// Thrown if a file found at cache path has an invalid format.
const CacheError NotCacheError("not a cache file");

const CacheError UpdatePathError("could not open file at the specified path");

} // namespace errors

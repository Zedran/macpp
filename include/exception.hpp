#pragma once

#include <curl/curl.h>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace errors {

class Error : public std::runtime_error {
public:
    explicit Error(const std::string& msg) : std::runtime_error{msg} {}

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
        : Error{msg + ": (" + std::to_string(sqlite3_errcode(conn)) + ") " + sqlite3_errmsg(conn)} {}

    // Returns a new CacheError that wraps sqlite3 error information retrieved
    // from SQLite response code.
    explicit CacheError(const std::string& msg, const int code)
        : Error{msg + ": (" + std::to_string(code) + ") " + sqlite3_errstr(code)} {}
};

class NetworkError : public Error {
public:
    explicit NetworkError(const std::string& msg) : Error{msg} {}

    // Constructs a new NetworkError that contains an error message retrieved
    // from CURL.
    explicit NetworkError(const std::string& msg, const CURLcode code)
        : Error{msg + ": (" + std::to_string(static_cast<int>(code)) + ") " + curl_easy_strerror(code)} {}
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
    explicit NoCommaError(const std::string& line) : ParsingError{"no comma found in CSV line", line} {}
};

// Thrown if quoted vendor name field is not terminated with '",' sequence,
// e.g. ',"Cisco Systems, Inc,false'
class QuotedTermSeqError : public ParsingError {
public:
    explicit QuotedTermSeqError(const std::string& line) : ParsingError{"closing '\",' sequence for vendor name not found", line} {}
};

// Thrown if unquoted vendor name field does not end with a comma
class UnquotedTermError : public ParsingError {
public:
    explicit UnquotedTermError(const std::string& line) : ParsingError{"no comma after unquoted vendor name", line} {}
};

class PrivateInvalidError : public ParsingError {
public:
    explicit PrivateInvalidError(const std::string& line) : ParsingError{"invalid value of private field", line} {}
};

class PrivateTermError : public ParsingError {
public:
    explicit PrivateTermError(const std::string& line) : ParsingError{"no comma between private and block type fields", line} {}
};

class BlockTypeTermError : public ParsingError {
public:
    explicit BlockTypeTermError(const std::string& line) : ParsingError{"no comma between block type and last update fields", line} {}
};

} // namespace errors

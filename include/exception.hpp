#pragma once

#include <curl/curl.h>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>
#include <string>

namespace errors {

// Base exception class for signaling general errors.
class Error : public std::runtime_error {
public:
    // Constructs a simple Error with a message.
    explicit Error(const std::string& msg) : std::runtime_error{msg} {}

    // Constructs a new Error and adds the origin function name to the message.
    explicit Error(const std::string& msg, const std::string& func)
        : std::runtime_error{"[ " + func + " ] " + msg} {}

    friend std::ostream& operator<<(std::ostream& os, const Error& e) {
        return os << e.what();
    }
};

// Exception class designed to embed information related to SQLite errors.
class CacheError : public Error {
public:
    explicit CacheError(const std::string& msg) : Error{msg} {}

    // Constructs a new CacheError that contains SQLite error information
    // retrieved from sqlite3 object. Adds the origin function name
    // to the message.
    explicit CacheError(const std::string& msg, const std::string& func, sqlite3* const conn)
        : Error{msg + ": (" + std::to_string(sqlite3_errcode(conn)) + ") " + sqlite3_errmsg(conn), func} {}

    // Constructs a new CacheError that contains SQLite error information
    // retrieved from SQLite response code. Adds the origin function name
    // to the message.
    explicit CacheError(const std::string& msg, const std::string& func, const int code)
        : Error{msg + ": (" + std::to_string(code) + ") " + sqlite3_errstr(code), func} {}
};

// Exception class designed to embed information related to CURL errors.
class UpdateError : public Error {
public:
    explicit UpdateError(const std::string& msg) : Error{msg} {}

    // Constructs a new NetworkError that contains error information retrieved
    // from CURLcode.
    explicit UpdateError(const std::string& msg, const CURLcode code)
        : Error{msg + ": (" + std::to_string(static_cast<int>(code)) + ") " + curl_easy_strerror(code)} {}
};

// Base exception class representing parsing errors. Parsing errors may occur
// due to malformed CSV line encountered by the Vendor constructor.
// ParsingError cannot be instantiated directly - its subclasses should always
// be used to allow strict testing of the parsing procedure.
class ParsingError : public Error {
    // CSV line that caused the exception.
    std::string line;

protected:
    explicit ParsingError(const std::string& msg, const std::string& line)
        : Error{msg}, line{line} {}

public:
    // Outputs the outcome of what() and the problematic line into os.
    // User interface should use this operator to display the error message.
    // what() function was not overridden to facilitate testing.
    friend std::ostream& operator<<(std::ostream& os, const ParsingError& e) {
        return os << e.what() << ": '" << e.line << '\'';
    }
};

// Thrown if CSV line does not contain any comma.
class NoCommaError : public ParsingError {
public:
    explicit NoCommaError(const std::string& line)
        : ParsingError{"no comma found in CSV line", line} {}
};

// Thrown if prefix field ends the line.
class PrefixTermError : public ParsingError {
public:
    explicit PrefixTermError(const std::string& line)
        : ParsingError{"prefix field ends the line", line} {}
};

// Thrown if quoted vendor name field is not terminated with '",' sequence,
// e.g. ',"Cisco Systems, Inc,false'
class QuotedTermSeqError : public ParsingError {
public:
    explicit QuotedTermSeqError(const std::string& line)
        : ParsingError{"closing '\",' sequence for vendor name not found", line} {}
};

// Thrown if unquoted vendor name field does not end with a comma
class UnquotedTermError : public ParsingError {
public:
    explicit UnquotedTermError(const std::string& line)
        : ParsingError{"no comma after unquoted vendor name", line} {}
};

// Thrown if private field value is neither 'true' nor 'false'.
class PrivateInvalidError : public ParsingError {
public:
    explicit PrivateInvalidError(const std::string& line)
        : ParsingError{"invalid value of private field", line} {}
};

// Thrown if private field is not terminated with a comma.
class PrivateTermError : public ParsingError {
public:
    explicit PrivateTermError(const std::string& line)
        : ParsingError{"no comma between private and block type fields", line} {}
};

// Thrown if block type field is not terminated with a comma.
class BlockTypeTermError : public ParsingError {
public:
    explicit BlockTypeTermError(const std::string& line)
        : ParsingError{"no comma between block type and last update fields", line} {}
};

} // namespace errors

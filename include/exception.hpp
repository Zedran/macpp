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
    using Error::Error;

public:
    // Returns a new CacheError that wraps sqlite3 error information retrieved
    // from sqlite3 object.
    CacheError wrap(sqlite3* const conn) const {
        if (conn) {
            return CacheError(std::format(
                "{}: ({}) {}",
                this->what(),
                sqlite3_errcode(conn),
                sqlite3_errmsg(conn)
            ));
        }
        return *this;
    }

    // Returns a new CacheError that wraps sqlite3 error information retrieved
    // from SQLite response code.
    CacheError wrap(const int code) const {
        return CacheError(std::format(
            "{}: ({}) {}",
            this->what(),
            code,
            sqlite3_errstr(code)
        ));
    }

    // Returns a new CacheError that wraps a custom message.
    CacheError wrap(const char* message) const {
        if (message) {
            return CacheError(std::format("{}: {}", this->what(), message));
        }
        return *this;
    }
};

class NetworkError : public Error {
    using Error::Error;

public:
    // Returns a new NetworkError that wraps an error message retrieved
    // from CURL.
    NetworkError wrap(const CURLcode code) const {
        return NetworkError(std::format(
            "{}: {}",
            this->what(),
            curl_easy_strerror(code)
        ));
    }
};

class ParsingError : public Error {
    using Error::Error;

public:
    // Returns a new ParsingError that wraps a problematic CSV line.
    ParsingError wrap(const std::string& line) const {
        return ParsingError(std::format("{}: in line '{}'", this->what(), line));
    }
};

// Thrown if --addr option is given with no value
const Error EmptyAddrError("empty MAC address");

// Thrown if --name option is given with no value
const Error EmptyNameError("empty vendor name");

// Thrown if no command is given (--addr, --name, --update).
const Error NoActionError("no action specified");

const CacheError BindError("sqlite3_bind error");
const CacheError CacheOpenError("failed to open the database");
const CacheError ExecError("sqlite3_exec error");

// Thrown if a file found at cache path has an invalid format.
const CacheError NotCacheError("not a cache file");

const CacheError PrepareError("sqlite3_prepare_v2 error");
const CacheError ResetError("sqlite3_reset error");
const CacheError StepError("sqlite3_step error");
const CacheError UpdateError("update failed");

const NetworkError EasyInitError("curl_easy_init failed");
const NetworkError FileSizeError("file size limit exceeded during download");
const NetworkError GlobalInitError("curl_global_init failed");
const NetworkError PerformError("curl_easy_perform failed");

const ParsingError NoCommaError("no comma found in CSV line");

// Thrown if escaped quote in vendor name field is not terminated,
// e.g. ',"IEE&E ""Black ops",'
const ParsingError EscapedTermError("closing escaped quote not found");

// Thrown if quoted vendor name field is not terminated with '",' sequence,
// e.g. ',"Cisco Systems, Inc,false'
const ParsingError QuotedTermSeqError("closing quote + comma sequence for vendor name not found");

// Thrown if unquoted vendor name field does not end with a comma
const ParsingError UnquotedTermError("no comma after unquoted vendor name");

const ParsingError PrivateInvalidError("invalid value of private field");
const ParsingError PrivateTermError("no comma between private and block type fields");
const ParsingError BlockTypeTermError("no comma between block type and last update fields");

} // namespace errors

#pragma once

#include <curl/curl.h>
#include <format>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>

namespace errors {

class Error : public std::runtime_error {
    using std::runtime_error::runtime_error;

public:
    bool operator==(const Error& other) const noexcept {
        return std::string(this->what()) == std::string(other.what());
    }

    bool operator!=(const Error& other) const noexcept {
        return std::string(this->what()) != std::string(other.what());
    }

    friend std::ostream& operator<<(std::ostream& os, const Error& e) {
        return os << e.what();
    }
};

class CacheError : public Error {
    using Error::Error;

public:
    // Returns a new CacheError that wraps sqlite3 error information.
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

} // namespace errors

#pragma once

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

// Thrown if no command is given (--addr, --name, --update).
const Error NoActionError("no action specified");

} // namespace errors

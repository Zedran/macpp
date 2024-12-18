#pragma once

#include <curl/curl.h>
#include <exception>
#include <sqlite3.h>
#include <string>

class AppError : public std::exception {
    std::string message;

public:
    AppError();
    AppError(const std::string message);

    // Wraps sqlite3_errcode and sqlite3_errmsg.
    AppError(const std::string message, sqlite3* conn);

    const char* what() const noexcept;
};

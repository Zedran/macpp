#include <cstring>
#include <curl/curl.h>
#include <format>
#include <string>

#include "AppError.hpp"

AppError::AppError() : message{"unknown error occured"} {}

AppError::AppError(const std::string message) : message(message) {}

AppError::AppError(const std::string message, sqlite3* conn) {
    if (conn) {
        this->message = std::format(
            "{}: ({}) {}",
            message,
            sqlite3_errcode(conn),
            sqlite3_errmsg(conn)
        );
    } else {
        this->message = message;
    }
}

const char* AppError::what() const noexcept {
    return message.c_str();
}

#include <cstring>
#include <curl/curl.h>
#include <format>
#include <string>

#include "AppError.hpp"

AppError::AppError() : message{"unknown error occured"} {}

AppError::AppError(const std::string message) : message(message) {}

AppError::AppError(const CURLcode code) {
    message = std::format("CURL error: {}", static_cast<int>(code));
}

const char* AppError::what() const noexcept {
    return message.c_str();
}

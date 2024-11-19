#include <cstring>
#include <curl/curl.h>
#include <string>

#include "AppError.hpp"

AppError::AppError() : message{"unknown error occured"} {}

AppError::AppError(const std::string message) : message(message) {}

const char* AppError::what() const noexcept {
    return message.c_str();
}

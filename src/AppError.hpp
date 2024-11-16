#pragma once

#include <curl/curl.h>
#include <exception>
#include <string>

class AppError : public std::exception {
    std::string message;

public:
    AppError();
    AppError(const std::string message);
    AppError(const CURLcode code);

    const char* what() const noexcept;
};

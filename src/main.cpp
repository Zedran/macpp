#include <curl/curl.h>
#include <iostream>

#include "AppError.hpp"
#include "cache.hpp"

int main() {
    try {
        create_cache("mac.db");
    } catch (const AppError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#include <curl/curl.h>
#include <iostream>

#include "AppError.hpp"
#include "Vendor.hpp"
#include "json.hpp"

int main() {
    std::vector<Vendor> data;

    try {
        data = download_data();
    } catch (const AppError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::cout << data[0] << std::endl;

    return 0;
}

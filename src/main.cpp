#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "AppError.hpp"
#include "json.hpp"

int main() {
    std::vector<char> json_data;
    nlohmann::json    data;

    try {
        data = download_json();
    } catch (const AppError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error:" << e.what() << std::endl;
        return 2;
    }

    std::cout << data[0]["macPrefix"] << std::endl;

    return 0;
}

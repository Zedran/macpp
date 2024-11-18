#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "AppError.hpp"
#include "utils.hpp"

// WRITEFUNCTION function for cURL.
size_t write_data(char* buffer, size_t size, size_t nmemb, std::vector<char>* userp) {
    try {
        userp->reserve(userp->size() + size * nmemb);
    } catch (const std::bad_alloc&) {
        return 0;
    }

    userp->insert(userp->end(), buffer, buffer + size * nmemb);
    return size * nmemb;
}

// Downloads and parses JSON data.
nlohmann::json download_json() {
    const char* URL = "https://maclookup.app/downloads/json-database/get-db";

    CURLcode          code;
    std::vector<char> stream;

    code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK) {
        throw(AppError("curl_global_init failed"));
    }

    auto global_cleanup = finally([&] { curl_global_cleanup(); });

    auto curl = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>(curl_easy_init(), &curl_easy_cleanup);
    if (!curl) {
        throw(AppError("curl_easy_init failed"));
    }

    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl.get(), CURLOPT_URL, URL);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &stream);

    code = curl_easy_perform(curl.get());

    if (code != CURLE_OK) {
        throw(AppError(curl_easy_strerror(code)));
    }

    return nlohmann::json::parse(stream);
}

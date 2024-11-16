#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "AppError.hpp"

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

    CURL*             curl;
    CURLcode          code;
    std::vector<char> stream;

    code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK) {
        throw(AppError("curl_global_init failed"));
    }

    curl = curl_easy_init();
    if (curl == nullptr) {
        curl_global_cleanup();
        throw(AppError("curl_easy_init failed"));
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);

    code = curl_easy_perform(curl);

    if (code != CURLE_OK) {
        AppError err = AppError(curl_easy_strerror(code));

        curl_easy_cleanup(curl);
        curl_global_cleanup();

        throw(err);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return nlohmann::json::parse(stream);
}

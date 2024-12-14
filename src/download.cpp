#include <curl/curl.h>
#include <fstream>
#include <memory>

#include "AppError.hpp"
#include "FinalAction.hpp"

// WRITEFUNCTION function for cURL.
size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp) {
    std::string* data = static_cast<std::string*>(userp);

    data->append(static_cast<char*>(buffer), size * nmemb);
    return size * nmemb;
}

std::string download_data() {
    const char* URL = "https://maclookup.app/downloads/csv-database/get-db";

    CURLcode    code;
    std::string readBuffer;

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
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &readBuffer);

    code = curl_easy_perform(curl.get());

    if (code != CURLE_OK) {
        throw(AppError(curl_easy_strerror(code)));
    }

    return readBuffer;
}

std::ifstream get_local_file(const std::string& path) {
    std::ifstream file(path);

    if (!file.good()) {
        throw(AppError("cannot open file at '" + path + "'"));
    }

    return file;
}

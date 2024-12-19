#include <curl/curl.h>
#include <fstream>

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
    CURL*       curl{};
    std::string readBuffer;

    code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK) {
        throw(AppError("curl_global_init failed"));
    }

    auto global_cleanup = finally([&] { curl_global_cleanup(); });

    if (!(curl = curl_easy_init())) {
        throw(AppError("curl_easy_init failed"));
    }

    auto easy_cleanup = finally([&] { curl_easy_cleanup(curl); });

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    code = curl_easy_perform(curl);

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

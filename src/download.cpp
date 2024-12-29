#include <curl/curl.h>

#include "AppError.hpp"
#include "FinalAction.hpp"
#include "download.hpp"
#include "exception.hpp"

// WRITEFUNCTION function for cURL.
size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp) {
    std::stringstream* data = static_cast<std::stringstream*>(userp);

    data->write(static_cast<char*>(buffer), size * nmemb);
    return size * nmemb;
}

std::stringstream download_data() {
    const char* URL = "https://maclookup.app/downloads/csv-database/get-db";

    constexpr int64_t MAX_FSIZE = 1 << 23; // 8 MiB

    CURLcode          code;
    CURL*             curl{};
    std::stringstream readBuffer;

    code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (code != CURLE_OK) {
        throw errors::GlobalInitError.wrap(code);
    }

    const auto global_cleanup = finally([&] { curl_global_cleanup(); });

    if (!(curl = curl_easy_init())) {
        throw errors::EasyInitError;
    }

    const auto easy_cleanup = finally([&] { curl_easy_cleanup(curl); });

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, MAX_FSIZE);

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    code = curl_easy_perform(curl);

    if (code == CURLE_FILESIZE_EXCEEDED) {
        throw errors::FileSizeError;
    } else if (code != CURLE_OK) {
        throw errors::PerformError.wrap(code);
    }

    return readBuffer;
}

std::fstream get_local_file(const std::string& path) {
    std::fstream file(path);

    if (!file.good()) {
        throw(AppError("cannot open file at '" + path + "'"));
    }

    return file;
}

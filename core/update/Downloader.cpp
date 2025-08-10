#include "update/Downloader.hpp"
#include "exception.hpp"

std::once_flag Downloader::curl_init{};

Downloader::Downloader(const std::string& url) {
    std::call_once(curl_init, [&] {
        if (const CURLcode rc = curl_global_init(CURL_GLOBAL_DEFAULT); rc != CURLE_OK) {
            throw errors::UpdateError{"curl_global_init failed", rc};
        }
    });

    if (!(curl = curl_easy_init())) {
        throw errors::UpdateError{"curl_easy_init failed"};
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, MAX_FSIZE);

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

    if (const CURLcode rc = curl_easy_perform(curl); rc != CURLE_OK) {
        curl_easy_cleanup(curl);

        if (rc == CURLE_FILESIZE_EXCEEDED) {
            throw errors::UpdateError{"file size limit exceeded during download"};
        }

        throw errors::UpdateError{"curl_easy_perform failed", rc};
    }
}

Downloader::~Downloader() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

std::istream& Downloader::get() noexcept {
    return data;
}

size_t Downloader::write_data(void* buffer, size_t size, size_t nmemb, void* userp) {
    std::stringstream* data = static_cast<std::stringstream*>(userp);

    data->write(static_cast<char*>(buffer), static_cast<std::streamsize>(size * nmemb));
    return size * nmemb;
}

#pragma once

#include <curl/curl.h>
#include <mutex>
#include <sstream>

#include "Updater.hpp"

// Class that provides data for cache update from a remote source.
class Downloader : public Updater {
    // Signals whether curl_global_init() function has been called.
    static std::once_flag curl_init;

    // CURL object.
    CURL* curl;

    // Data retrieved from source.
    std::stringstream data;

    // WRITEFUNCTION function for CURL.
    static size_t write_data(void* buffer, size_t size, size_t nmemb, void* userp);

public:
    // Constructs a new Downloader instance and downloads data from url.
    Downloader(const std::string& url);

    Downloader(const Downloader&)            = delete;
    Downloader& operator=(const Downloader&) = delete;

    Downloader(Downloader&& other);
    Downloader& operator=(Downloader&& other);

    ~Downloader();

    // Returns a reference to the wrapped stream.
    std::istream& get() noexcept override final;
};

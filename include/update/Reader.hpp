#pragma once

#include <fstream>

#include "Updater.hpp"

// Class that provides data for cache update from a local file.
class Reader : public Updater {
    std::ifstream file;

public:
    // Constructs a new Reader instance and opens a file stream at path.
    Reader(const std::string& path);

    Reader(const Reader&)            = delete;
    Reader& operator=(const Reader&) = delete;

    // Returns a reference to the wrapped stream.
    std::istream& get() noexcept override final;
};

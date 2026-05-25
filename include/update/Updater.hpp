#pragma once

#include <cstddef>
#include <iostream>

// Common interface for Downloader and Reader classes.
class Updater {
public:
    // Maximum file size that is allowed to be processed by the application.
    // Set at 16 MiB.
    static constexpr size_t MAX_FSIZE = 1 << 24;

    virtual ~Updater() = default;

    // Returns a reference to stream wrapped by derived class.
    virtual std::istream& get() noexcept = 0;
};

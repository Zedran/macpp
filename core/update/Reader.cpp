#include <filesystem>

#include "exception.hpp"
#include "update/Reader.hpp"

Reader::Reader(const std::string& path) : file{std::ifstream{path}} {
    if (!file.good()) {
        throw errors::Error{"file '" + path + "' not found"};
    }

    if (std::filesystem::file_size(path) > MAX_FSIZE) {
        throw errors::UpdateError{"file '" + path + "' exceeds local file size limit"};
    }
}

std::istream& Reader::get() noexcept {
    return file;
}

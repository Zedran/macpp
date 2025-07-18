#include <filesystem>

#include "dir.hpp"
#include "exception.hpp"

// Returns a system-dependent directory path for the application's cache file.
std::filesystem::path set_cache_dir_path() {
    std::filesystem::path cache_dir{};

    char* env_var{};

#if defined(__APPLE__)
    env_var = getenv("HOME");
#elif defined(__linux__)
    bool xdg_var_set = true;

    env_var = getenv("XDG_CACHE_HOME");
    if (!env_var) {
        env_var     = getenv("HOME");
        xdg_var_set = false;
    }
#elif defined(_WIN32)
    env_var = getenv("APPDATA");
#endif

    if (!env_var) {
        throw errors::Error{"could not resolve cache path"};
    }

    cache_dir = env_var;

#if defined(__APPLE__)
    cache_dir = cache_dir / "Library" / "Caches";
#elif defined(__linux__)
    if (!xdg_var_set) {
        cache_dir /= ".cache";
    }
#endif

    return cache_dir / "github.com" / "Zedran" / "mac";
}

std::string prepare_cache_dir() {
    const std::string cache_fname = "mac.db";

    auto cache_dir = set_cache_dir_path();

    std::filesystem::create_directories(cache_dir);

    return (cache_dir / cache_fname).string();
}

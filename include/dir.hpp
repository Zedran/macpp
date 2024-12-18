#pragma once

#include <string>

// Returns a system-dependent directory path for the application's cache file.
// Creates missing directories.
std::string prepare_cache_dir();

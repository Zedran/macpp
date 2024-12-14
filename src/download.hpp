#pragma once

#include <string>

// Downloads the CSV file and returns the result of parse_csv.
std::string download_data();

// Open local file containing CSV data.
std::ifstream get_local_file(const std::string& path);

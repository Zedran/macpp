#pragma once

#include <fstream>
#include <sstream>
#include <string>

// Downloads the CSV file and returns the result of parse_csv.
std::stringstream download_data();

// Open local file containing CSV data.
std::fstream get_local_file(const std::string& path);

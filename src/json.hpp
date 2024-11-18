#pragma once

#include <vector>

#include "Vendor.hpp"

// Downloads the CSV file and returns the result of parse_csv.
std::vector<Vendor> download_data();

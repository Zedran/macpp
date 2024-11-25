#pragma once

#include <vector>

#include "Vendor.hpp"

// Opens a new sqlite3 connection to the cached vendor database.
// If the file does not exist, a new one is created and populated with entries.
// If the file exists and is empty, a new database is written to it.
// If the file exists, is not empty and does not appear to be a SQLite
// database, an exception is returned.
void get_conn(sqlite3*& conn, const std::string& cache_path);

std::vector<Vendor> query_addr(sqlite3* conn, const std::string& address);

std::vector<Vendor> query_name(sqlite3* conn, const std::string& vendor_name);

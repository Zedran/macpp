#pragma once

#include <vector>

#include "Vendor.hpp"

// Opens a new sqlite3 connection to the cached vendor database.
// If the file does not exist, a new one is created and populated with entries.
// If the file exists and is empty, a new database is written to it.
// If the file exists, is not empty and does not appear to be a SQLite
// database, an exception is returned.
// For the update task, non-empty update_fpath is used as a local CSV file path.
void get_conn(sqlite3*& conn, const std::string& cache_path, const std::string& update_fpath = "");

std::vector<Vendor> query_addr(sqlite3* conn, const std::string& address);

std::vector<Vendor> query_name(sqlite3* conn, const std::string& vendor_name);

// Updates the database file at cache_path. Preserves the most recently
// created database. If an update fails, the most recent database is restored.
// If update_fpath is not empty a local CSV file it points to is used
// for the update.
void update_cache(sqlite3*& conn, const std::string& cache_path, const std::string& update_fpath = "");

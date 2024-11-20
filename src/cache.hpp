#pragma once

#include <vector>

#include "Vendor.hpp"

void get_conn(sqlite3*& conn);

std::vector<Vendor> query_addr(sqlite3* conn, const std::string& address);

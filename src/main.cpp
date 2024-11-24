#include <curl/curl.h>
#include <iostream>

#include "AppError.hpp"
#include "FinalAction.hpp"
#include "argparse/argparse.hpp"
#include "cache.hpp"

void setup_parser(argparse::ArgumentParser& app);

int main(int argc, char* argv[]) {
    argparse::ArgumentParser app("macpp");
    setup_parser(app);

    try {
        app.parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return -2;
    }

    if (sqlite3_initialize() != SQLITE_OK) {
        std::cerr << "failed to initialize sqlite" << '\n';
        return -1;
    }

    sqlite3* conn{};

    auto cleanup = finally([&] {
        if (sqlite3_close(conn) != SQLITE_OK) {
            std::cerr << "failed to close connection" << '\n';
        }
        if (sqlite3_shutdown() != SQLITE_OK) {
            std::cerr << "failed to shutdown sqlite" << '\n';
        }
    });

    std::vector<Vendor> results;

    try {
        get_conn(conn);

        if (app.is_used("--addr"))
            results = query_addr(conn, app.get("--addr"));
        else if (app.is_used("--name"))
            results = query_name(conn, app.get("--name"));
        else
            throw(AppError("no action specified"));
    } catch (const AppError& e) {
        std::cerr << e.what() << '\n';
        return 1;
    } catch (const std::invalid_argument& e) {
        std::cerr << "invalid address" << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 2;
    }

    if (results.empty()) {
        std::cout << "no matches found\n";
    } else {
        for (const auto& v : results)
            std::cout << v << "\n\n";
    }

    return 0;
}

void setup_parser(argparse::ArgumentParser& app) {
    app.add_description("A simple tool for MAC address lookup.");
    app.set_usage_max_line_width(80);

    app.add_argument("-a", "--addr")
        .help("Search by MAC address (e.g. \"000000\", \"01:23:45:67:89:01\").")
        .metavar("ADDR");
    app.add_argument("-n", "--name")
        .help("Search by vendor name (e.g. \"xerox\", \"xerox corporation\").")
        .metavar("NAME");
}

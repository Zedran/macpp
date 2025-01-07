#include <curl/curl.h>
#include <iostream>

#include "FinalAction.hpp"
#include "argparse/argparse.hpp"
#include "cache.hpp"
#include "config.hpp"
#include "dir.hpp"
#include "exception.hpp"

void setup_parser(argparse::ArgumentParser& app);

int main(int argc, char* argv[]) {
    argparse::ArgumentParser app("macpp", std::string(VERSION_INFO));
    setup_parser(app);

    try {
        app.parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    if (sqlite3_initialize() != SQLITE_OK) {
        std::cerr << "failed to initialize sqlite\n";
        return 1;
    }

    sqlite3* conn{};

    const auto cleanup = finally([&] {
        if (sqlite3_close(conn) != SQLITE_OK) {
            std::cerr << "failed to close connection\n";
        }
        if (sqlite3_shutdown() != SQLITE_OK) {
            std::cerr << "failed to shutdown sqlite\n";
        }
    });

    std::vector<Vendor> results;

    try {
        const std::string cache_path = prepare_cache_dir();

        if (app.is_used("--update")) {
            std::string update_fpath{};

            if (app.is_used("--file")) {
                update_fpath = app.get("--file");
            }
            update_cache(conn, cache_path, update_fpath);
            return 0;
        }

        get_conn(conn, cache_path);

        if (app.is_used("--addr"))
            results = query_addr(conn, app.get("--addr"));
        else if (app.is_used("--name"))
            results = query_name(conn, app.get("--name"));
        else
            throw errors::NoActionError;
    } catch (const errors::Error& e) {
        std::cerr << e << '\n';
        return 1;
    } catch (const std::invalid_argument&) {
        // Thrown by std::stoll in prefix_to_id
        std::cerr << errors::AddrInvalidError << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "unexpected error: " << e.what() << '\n';
        return 1;
    }

    if (results.empty()) {
        std::cout << "no matches found\n";
    } else {
        for (auto it = results.begin(); it != results.end(); it++)
            std::cout << *it << (std::next(it) == results.end() ? "\n" : "\n\n");
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
    app.add_argument("-u", "--update")
        .help("Update vendor database and exit.")
        .flag();
    app.add_argument("-f", "--file")
        .help("Use a local file instead of downloading one during update.")
        .metavar("PATH");
}

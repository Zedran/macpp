#include <curl/curl.h>
#include <iostream>
#include <optional>

#include "FinalAction.hpp"
#include "argparse/argparse.hpp"
#include "cache/ConnR.hpp"
#include "cache/ConnRW.hpp"
#include "config.hpp"
#include "dir.hpp"
#include "exception.hpp"
#include "out.hpp"
#include "update/Downloader.hpp"
#include "update/Reader.hpp"

// Presents results in the user-specified (or default) format.
void display_results(const argparse::ArgumentParser& app, const std::vector<Vendor>& results) {
    const std::string format = (app.is_used("--out-format") ? app.get("--out-format") : "regular");

    if (format == "regular") {
        for (auto it = results.begin(); it != results.end(); it++) {
            std::cout << *it << (std::next(it) == results.end() ? "\n" : "\n\n");
        }
        return;
    }

    if (format == "csv") {
        std::cout << "MAC Prefix,Vendor Name,Private,Block Type,Last Update\n"
                  << out::csv;
        for (const auto& v : results) {
            std::cout << v << '\n';
        }
        return;
    }

    if (format == "json") {
        std::cout << '[' << out::json;
        for (auto it = results.begin(); it != results.end(); it++) {
            std::cout << *it << (std::next(it) == results.end() ? "]\n" : ",");
        }
        return;
    }

    if (format == "xml") {
        std::cout << R"(<MacAddressVendorMappings xmlns="http://www.cisco.com/server/spt">)"
                  << out::xml;
        for (const auto& v : results) {
            std::cout << "\n\t" << v;
        }
        std::cout << "\n</MacAddressVendorMappings>\n";
        return;
    }

    throw errors::Error{"unknown output format '" + format + '\''};
}

// Updates cache at the specified db_path. If update_path holds string, the function
// will update the database from local file instead of downloading data.
void update(const std::string& db_path, const std::optional<std::string>& update_fpath) {
    ConnRW conn{db_path};

    if (!update_fpath) {
        const auto cleanup = finally([] { curl_global_cleanup(); });
        conn.insert(Downloader{"https://maclookup.app/downloads/csv-database/get-db"}.get());
    } else {
        conn.insert(Reader{*update_fpath}.get());
    }
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser app{"macpp", VERSION_INFO};
    app.add_description("Tool for MAC address lookup.\nData source: https://maclookup.app.");
    app.set_usage_max_line_width(80);
    app.add_argument("-o", "--out-format")
        .help("display found entries in the chosen format: 'csv', 'json' or 'regular'")
        .metavar("FORMAT");

    argparse::ArgumentParser sc_addr{"addr"};
    sc_addr.add_description("Search by MAC address.");
    sc_addr.add_argument("addr")
        .help("MAC address (e.g. \"000000\", \"01:23:45:67:89:01\").")
        .remaining();
    app.add_subparser(sc_addr);

    argparse::ArgumentParser sc_name{"name"};
    sc_name.add_description("Search by vendor name.");
    sc_name.add_argument("name")
        .help("Vendor name (e.g. \"xerox\", \"xerox corporation\").")
        .remaining();
    app.add_subparser(sc_name);

    argparse::ArgumentParser sc_update{"update"};
    sc_update.add_description("Update vendor database and exit.");
    sc_update.add_argument("-f", "--file")
        .help("Use a local file instead of downloading one during update.")
        .metavar("PATH");
    app.add_subparser(sc_update);

    const auto cleanup = finally([] {
        if (sqlite3_shutdown() != SQLITE_OK) {
            std::cerr << "failed to shutdown sqlite\n";
        }
    });

    try {
        app.parse_args(argc, argv);

        const std::string cache_path = prepare_cache_dir();

        if (app.is_subcommand_used(sc_update)) {
            std::optional<std::string> update_fpath{std::nullopt};

            if (sc_update.is_used("--file")) {
                update_fpath = sc_update.get<std::string>("--file");
            }
            update(cache_path, update_fpath);
            return 0;
        }

        const ConnR conn{cache_path};

        if (app.is_subcommand_used(sc_addr)) {
            display_results(app, conn.find_by_addr(sc_addr.get<std::string>("addr")));
        } else if (app.is_subcommand_used(sc_name)) {
            display_results(app, conn.find_by_name(sc_name.get<std::string>("name")));
        } else {
            throw errors::Error{"no action specified"};
        }
    } catch (const errors::Error& e) {
        std::cerr << e << '\n';
        return 1;
    } catch (const std::runtime_error& e) {
        // Expected Argparse parsing errors
        std::cerr << e.what() << '\n';
        return 1;
    } catch (const std::logic_error& e) {
        // Expected Argparse getter errors
        std::cerr << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "unexpected error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

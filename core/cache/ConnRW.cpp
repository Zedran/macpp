#include <array>
#include <cassert>

#include "Registry.hpp"
#include "Vendor.hpp"
#include "cache/ConnRW.hpp"
#include "cache/Stmt.hpp"
#include "exception.hpp"
#include "utils.hpp"

std::once_flag ConnRW::db_prepared{};

ConnRW::ConnRW(const std::string& path, const bool override_once_flags)
    : Conn(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE),
      override_once_flags{override_once_flags},
      transaction_open{false} {
    if (sqlite_open_rc != SQLITE_OK) {
        throw errors::CacheError{"open", __func__, sqlite_open_rc};
    }

    if (!override_once_flags) [[likely]] {
        std::call_once(db_prepared, [&] { prepare_db(); });
    } else [[unlikely]] {
        prepare_db();
    }
}

ConnRW::~ConnRW() {
    if (transaction_open) {
        [[maybe_unused]] const int rc = rollback();
        assert(rc == SQLITE_OK);
    }
}

void ConnRW::begin() {
    assert(!transaction_open && "transaction already open");

    exec("BEGIN");
    transaction_open = true;
}

void ConnRW::clear_table() {
    exec("DELETE FROM vendors");
}

void ConnRW::commit() {
    exec("COMMIT");
    transaction_open = false;
}

void ConnRW::create_table() {
    exec(CREATE_TABLE_STMT);
}

void ConnRW::customize_db(std::ostream& err) {
    constexpr std::array<std::string_view, 2> mods{
        "UPDATE vendors SET name = 'QEMU/KVM' WHERE prefix = 0x525400",
        "UPDATE vendors SET name = name || ' (VirtualBox)' WHERE prefix = 0x080027",
    };

    for (const auto& stmt : mods) {
        exec(stmt);

        int changes = sqlite3_changes(conn);
        if (changes != 1) {
            err << "[ " << __func__ << " ] " << stmt << ": unexpected number of changes (" << changes << ")\n";
        }
    }

    Stmt ins{conn, INSERT_STMT};

    try {
        ins.insert_row(Vendor{0x024200, "Docker container interface (02:42)", true, Registry::Unknown, ""});
    } catch (const errors::CacheError& e) {
        err << e << '\n';
        try {
            ins.reset();
        } catch (const errors::CacheError&) {
            // Ignore exception.
            // At this point, reset can throw due to (19) constraint failed,
            // which is expected if a record is already present.
        }
    }
}

void ConnRW::drop_table() {
    exec("DROP TABLE IF EXISTS vendor");
}

void ConnRW::exec(std::string_view stmt_str, const std::source_location loc) {
    Stmt stmt{conn, stmt_str.data()};

    if (const int rc = stmt.step(); rc != SQLITE_OK && rc != SQLITE_DONE) {
        throw errors::CacheError{stmt_str.data(), fmt_loc(loc), rc};
    }
}

void ConnRW::insert(std::istream& is, const bool update, std::ostream& err) {
    begin();

    if (update) {
        clear_table();
    }

    Stmt stmt{conn, INSERT_STMT};

    std::string line;

    // Discard the header line
    std::getline(is, line);

    while (std::getline(is, line)) {
        if (!line.empty() && line.length() <= MAX_LINE_LENGTH) {
            stmt.insert_row(Vendor{line});
        }
    }

    if (update) {
        customize_db(err);
    }

    commit();
}

void ConnRW::prepare_db() {
    bool needs_table;

    if (version() != EXPECTED_CACHE_VERSION) {
        drop_table();
        needs_table = true;
        set_version(EXPECTED_CACHE_VERSION);
    } else {
        needs_table = !has_table();
    }

    if (needs_table) {
        create_table();
    }
}

int ConnRW::rollback() noexcept {
    assert(transaction_open && "transaction has already been comitted");

    int rc = sqlite3_exec(conn, "ROLLBACK", nullptr, nullptr, nullptr);
    if (rc == SQLITE_OK) {
        transaction_open = false;
    }
    return rc;
}

void ConnRW::set_version(const int version) {
    exec("PRAGMA user_version = " + std::to_string(version));
}

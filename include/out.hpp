#pragma once

#include <iostream>

namespace out {

static int xindex = std::ios_base::xalloc();

// Signals to operator<< which Vendor data output format is currently selected.
enum class Format {
    // Regular, human-readable output.
    Regular,

    // CSV line, similar to the original data source.
    CSV,

    // JSON dictionary, similar to the one offered by the original data source.
    JSON,

    // XML, compliant with Cisco PI vendorMacs.
    XML,
};

// Sets Vendor data output format to CSV.
std::ostream& csv(std::ostream& os);

// Returns currently set output format for os.
Format get_format(std::ostream& os);

// Sets Vendor data output format to JSON.
std::ostream& json(std::ostream& os);

// Sets Vendor data output format to Regular.
std::ostream& regular(std::ostream& os);

// Sets Vendor data output format to XML.
std::ostream& xml(std::ostream& os);

} // namespace out

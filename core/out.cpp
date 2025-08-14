#include "out.hpp"

namespace out {

std::ostream& csv(std::ostream& os) {
    os.iword(xindex) = static_cast<long>(Format::CSV);
    return os;
}

Format get_format(std::ostream& os) {
    return static_cast<Format>(os.iword(xindex));
}

std::ostream& json(std::ostream& os) {
    os.iword(xindex) = static_cast<long>(Format::JSON);
    return os;
}

std::ostream& regular(std::ostream& os) {
    os.iword(xindex) = static_cast<long>(Format::Regular);
    return os;
}

} // namespace out

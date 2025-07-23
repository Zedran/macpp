#include "Registry.hpp"

std::string from_registry(const Registry registry) noexcept {
    using enum Registry;

    switch (registry) {
    case MA_L: return "MA-L";
    case MA_S: return "MA-S";
    case MA_M: return "MA-M";
    case IAB:  return "IAB";
    case CID:  return "CID";
    default:   return "-";
    }
}

Registry to_registry(const std::string& registry) noexcept {
    using enum Registry;

    if (registry == "MA-L") {
        return MA_L;
    }
    if (registry == "MA-S") {
        return MA_S;
    }
    if (registry == "MA-M") {
        return MA_M;
    }
    if (registry == "IAB") {
        return IAB;
    }
    if (registry == "CID") {
        return CID;
    }
    return Unknown;
}

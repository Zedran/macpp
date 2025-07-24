#pragma once

#include <string>

// Represents registry to which the assigned MAC address block belongs.
enum class Registry {
    Unknown,
    CID,
    IAB,
    MA_L,
    MA_M,
    MA_S,
};

// Converts Registry value to its string representation.
std::string from_registry(const Registry registry) noexcept;

// Converts string to Registry value.
Registry to_registry(const std::string& registry) noexcept;

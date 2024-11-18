#include <algorithm>
#include <cstdint>
#include <string>

int64_t prefix_to_id(const std::string& prefix) {
    std::string s = prefix;
    s.erase(std::remove(s.begin(), s.end(), ':'), s.end());

    return std::stoll(s, nullptr, 16);
}

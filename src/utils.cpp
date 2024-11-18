#include <algorithm>
#include <cstdint>
#include <string>

int64_t prefix_to_id(const std::string& prefix) {
    std::string s = prefix;
    s.erase(std::remove(s.begin(), s.end(), ':'), s.end());

    return std::stoll(s, nullptr, 16);
}

void replace_escaped_quotes(std::string& str) {
    const std::string target      = "\"\"";
    const std::string replacement = "\"";

    size_t pos = 0;

    while ((pos = str.find(target, pos)) != std::string::npos) {
        str.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }
}

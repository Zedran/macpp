#pragma once

#include <cstdint>
#include <string>

template <class F>
struct FinalAction {
    F act;

    explicit FinalAction(F f) : act(f) {}
    ~FinalAction() { act(); };
};

template <class F>
[[nodiscard]] auto finally(F f) {
    return FinalAction{f};
}

// Converts MAC prefix (10:10:10) from string to an integer.
int64_t prefix_to_id(const std::string& prefix);

// Removes "" (CSV escaped quotes) from str.
void replace_escaped_quotes(std::string& str);

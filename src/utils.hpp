#pragma once

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

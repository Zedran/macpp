#pragma once

#include <iostream>
#include <stdexcept>

namespace errors {

class Error : public std::runtime_error {
    using std::runtime_error::runtime_error;

public:
    friend std::ostream& operator<<(std::ostream& os, const Error& e) {
        return os << e.what();
    }
};

// Thrown if no command is given (--addr, --name, --update).
const Error NoActionError("no action specified");

} // namespace errors

#pragma once

#include <stdexcept>
#include <string>

namespace conf {
class BindingException : public std::exception {

public:
    explicit BindingException(std::string message): mMessage(std::move(message)) {}

    char const* what() const noexcept override {
        return mMessage.c_str();
    }
private:
    std::string mMessage;
};
}
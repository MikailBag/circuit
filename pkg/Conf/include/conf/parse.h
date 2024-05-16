#pragma once

#include <exception>
#include <memory>
#include <string_view>

namespace conf {


namespace detail {
class ParseExceptionAccess;
}

class Target;

class ParseException final : public std::exception {
    friend class detail::ParseExceptionAccess;
public:
    ParseException(ParseException&& that) = default;
    ParseException& operator=(ParseException&& that) = default;

    char const* what() const noexcept override;
    ~ParseException();

private:
    class Impl;
private:
    ParseException(std::unique_ptr<Impl> impl);
private:
    std::unique_ptr<Impl> mImpl;
};

void Parse(std::string_view data, Target& t);
}
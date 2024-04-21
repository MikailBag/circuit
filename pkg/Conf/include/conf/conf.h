#pragma once

#include "conf/validation.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace conf {
namespace detail {
class ParseExceptionAccess;
}


class Target;

class Description {
public:
    // object style
    virtual void IsObject() = 0;
    virtual void ObjField(std::string_view name, Target& f) = 0;
    virtual void BoolField(std::string_view name, bool& f) = 0;
    // enum style
    virtual void IsEnum() = 0;
    virtual void Variant(std::string_view name, bool& flag, Target& value) = 0;
    virtual ~Description();
};



class Target {
public:
    virtual void Describe(Description& desc) = 0;

    // throws BindingException
    virtual void Postprocess() = 0;

    virtual ~Target();
};

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
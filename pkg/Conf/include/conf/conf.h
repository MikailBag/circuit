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


// object style
class ObjectDescription {
public:
    virtual ObjectDescription& ObjField(std::string_view name, Target& f) = 0;
    virtual ObjectDescription& BoolField(std::string_view name, bool& f) = 0;
    virtual ObjectDescription& NumField(std::string_view name, size_t& f) = 0;
    
    virtual ~ObjectDescription();
};


// enum style
class EnumDescription {
public:
    virtual EnumDescription& Variant(std::string_view name, bool& flag, Target& value) = 0;
    virtual EnumDescription& SimpleVariant(std::string_view name, bool& flag) = 0;

    virtual ~EnumDescription();
};


class Description {
public:
    virtual ObjectDescription& Object() = 0;

    virtual EnumDescription& Enum() = 0;

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
#pragma once

#include <cstdint>
#include <string_view>

namespace conf {

class Target;


// object style
class ObjectDescription {
public:
    virtual ObjectDescription& ObjField(std::string_view name, Target& f) = 0;
    virtual ObjectDescription& BoolField(std::string_view name, bool& f) = 0;
    #define X(ty)  virtual ObjectDescription& NumField(std::string_view name, ty& f) = 0;
    #include "conf/supported_int_types.inc.x"
    #undef X
    
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

}
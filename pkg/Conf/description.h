#pragma once

#include "conf/target.h"

#include <charconv>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <variant>

namespace conf {
class Desc;

struct BoolDesc {
    bool* field;
};

struct NumDesc {
    std::function<std::from_chars_result(std::string_view)> parser;
    #define X(ty)  explicit NumDesc(ty& field);
    #include "conf/supported_int_types.inc.x"
    #undef X
};

// using ErasedPostprocessFunc = void(*)(void*);

struct ObjDesc {
    Target* ref;
    std::map<std::string, Desc> fields;
};

struct EnumVariant {
    bool* flag;
    // TODO: use unique_ptr
    std::shared_ptr<Desc> content;
};

struct EnumDesc {
    Target* ref;
    std::map<std::string, EnumVariant> variants;
};

class Desc {
public:
    Desc(ObjDesc&& od);
    Desc(EnumDesc&& ed);
    Desc(BoolDesc&& bd);
    Desc(NumDesc&& nd);
    bool IsBool() const;
    bool IsObj() const;
    bool IsEnum() const;
    bool IsNum() const;
    ObjDesc& AsObj();
    EnumDesc& AsEnum();
    BoolDesc& AsBool();
    NumDesc& AsNum();
private:
    std::variant<BoolDesc, ObjDesc, EnumDesc, NumDesc> mV;
};

Desc Describe(Target& t);
}
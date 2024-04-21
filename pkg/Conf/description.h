#pragma once

#include "conf/conf.h"

#include <map>
#include <string>
#include <variant>

namespace conf {
class Desc;

struct BoolDesc {
    bool* field;
};

using ErasedPostprocessFunc = void(*)(void*);

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
    Desc(BoolDesc&& bd);
    Desc(ObjDesc&& od);
    Desc(EnumDesc&& ed);
    bool IsBool() const;
    bool IsObj() const;
    bool IsEnum() const;
    ObjDesc& AsObj();
    BoolDesc& AsBool();
    EnumDesc& AsEnum();
private:
    std::variant<BoolDesc, ObjDesc, EnumDesc> mV;
};

Desc Describe(Target& t);
}
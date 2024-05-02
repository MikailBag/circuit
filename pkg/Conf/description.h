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

struct SizeTDesc {
    size_t* field;
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
    Desc(SizeTDesc&& sd);
    bool IsBool() const;
    bool IsObj() const;
    bool IsEnum() const;
    bool IsSizeT() const;
    ObjDesc& AsObj();
    EnumDesc& AsEnum();
    BoolDesc& AsBool();
    SizeTDesc& AsSizeT();
private:
    std::variant<BoolDesc, ObjDesc, EnumDesc, SizeTDesc> mV;
};

Desc Describe(Target& t);
}
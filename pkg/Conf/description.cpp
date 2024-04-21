#include "description.h"

#include <optional>
#include <stdexcept>
#include <variant>

namespace conf {
Desc::Desc(BoolDesc&& bd) : mV(std::move(bd)) {}
Desc::Desc(ObjDesc&& od) : mV(std::move(od)) {}
Desc::Desc(EnumDesc&& ed) : mV(std::move(ed)) {}

bool Desc::IsBool() const {
    return std::holds_alternative<BoolDesc>(mV);
}

bool Desc::IsObj() const {
    return std::holds_alternative<ObjDesc>(mV);
}

bool Desc::IsEnum() const {
    return std::holds_alternative<EnumDesc>(mV);
}

ObjDesc& Desc::AsObj() {
    if (!IsObj()) {
        throw std::runtime_error("Object-describing method used for non-object target");
    }
    return std::get<ObjDesc>(mV);
}

BoolDesc& Desc::AsBool() {
    if (!IsBool()) {
        throw std::runtime_error("Bool-describing method used for non-bool target");
    }
    return std::get<BoolDesc>(mV);
}

EnumDesc& Desc::AsEnum() {
    if (!IsEnum()) {
        throw std::runtime_error("Enum-describing method used for non-enum target");
    }
    return std::get<EnumDesc>(mV);
}

namespace {
class DescColl : public Description {
public:
    Desc Unpack() {
        if (!mDesc) {
            throw std::runtime_error("No Is* method was called");
        }
        return *mDesc;
    }

    void Bind(Target* ref) {
        CheckInit();
        if (mDesc->IsObj()) {
            mDesc->AsObj().ref = ref;
        } else if (mDesc->IsEnum()) {
            mDesc->AsEnum().ref = ref;
        } else if (mDesc->IsBool()) {
        } else {
            std::abort();
        }
    }

    void IsObject() override {
        CheckUninit();
        mDesc.emplace(ObjDesc{});
    }

    void ObjField(std::string_view name, Target& f) override {
        FieldCommon(name, Describe(f));
    }

    void BoolField(std::string_view name, bool& f) override {
        BoolDesc d;
        d.field = &f;
        FieldCommon(name, std::move(d));   
    }

    void IsEnum() override {
        CheckUninit();
        mDesc.emplace(EnumDesc{});
    }

    void Variant(std::string_view name, bool& flag, Target& value) override {
        CheckInit();
        std::string n {name};
        EnumDesc& ed = mDesc->AsEnum();
        if (ed.variants.contains(n)) {
            throw std::runtime_error("variant " + n + " was already registered");
        }
        EnumVariant var;
        var.flag = &flag;
        var.content = std::unique_ptr<Desc>{new Desc{Describe(value)}};

        ed.variants.emplace(std::move(n), std::move(var));
    }
private:
    void CheckInit() {
        if (!mDesc) {
            throw std::runtime_error("Is* method must be called before others");
        }
    }
    void CheckUninit() {
        if (mDesc) {
            throw std::runtime_error("Is* method may only be called once");
        }
    }
    void FieldCommon(std::string_view name, Desc fieldDesc) {
        CheckInit();
        ObjDesc& od = mDesc->AsObj();
        std::string n {name};
        if (od.fields.contains(n)) {
            throw std::runtime_error("field " + n + " was already registered");
        }
        od.fields.emplace(n, std::move(fieldDesc));
    }


private:
    std::optional<Desc> mDesc;
};
}


Desc Describe(Target& t) {
    DescColl coll;
    t.Describe(coll);
    coll.Bind(&t);
    return coll.Unpack();
}

}
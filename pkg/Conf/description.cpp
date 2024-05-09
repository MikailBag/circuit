#include "description.h"

#include <optional>
#include <stdexcept>
#include <variant>

namespace conf {
Desc::Desc(BoolDesc&& bd) : mV(std::move(bd)) {}
Desc::Desc(ObjDesc&& od) : mV(std::move(od)) {}
Desc::Desc(EnumDesc&& ed) : mV(std::move(ed)) {}
Desc::Desc(SizeTDesc&& sd) : mV(std::move(sd)) {}

bool Desc::IsBool() const {
    return std::holds_alternative<BoolDesc>(mV);
}

bool Desc::IsSizeT() const {
    return std::holds_alternative<SizeTDesc>(mV);
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

SizeTDesc& Desc::AsSizeT() {
    if (!IsSizeT()) {
        throw std::runtime_error("Integer-describing method used for non-numeric target");
    }
    return std::get<SizeTDesc>(mV);
}

namespace {
class DescColl : public Description, public ObjectDescription, public EnumDescription {
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

    ObjectDescription& Object() override {
        CheckUninit();
        mDesc.emplace(ObjDesc{});
        return *this;
    }

    ObjectDescription& ObjField(std::string_view name, Target& f) override {
        FieldCommon(name, Describe(f));
        return *this;
    }

    ObjectDescription& BoolField(std::string_view name, bool& f) override {
        BoolDesc d;
        d.field = &f;
        FieldCommon(name, std::move(d));   
        return *this;
    }

    ObjectDescription& NumField(std::string_view name, size_t& f) override {
        SizeTDesc d;
        d.field = &f;
        FieldCommon(name, std::move(d));
        return *this;
    }

    EnumDescription& Enum() override {
        CheckUninit();
        mDesc.emplace(EnumDesc{});
        return *this;
    }

    EnumDescription& Variant(std::string_view name, bool& flag, Target& value) override {
        EnumVariant var;
        var.flag = &flag;
        var.content = std::unique_ptr<Desc>{new Desc{Describe(value)}};
        VariantCommon(name, std::move(var));
        return *this;
    }

    EnumDescription& SimpleVariant(std::string_view name, bool& flag) override {
        EnumVariant var;
        var.flag = &flag;
        VariantCommon(name, std::move(var));
        return *this;
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
    void FieldCommon(std::string_view name, Desc&& fieldDesc) {
        CheckInit();
        ObjDesc& od = mDesc->AsObj();
        std::string n {name};
        if (od.fields.contains(n)) {
            throw std::runtime_error("field " + n + " was already registered");
        }
        od.fields.emplace(n, std::move(fieldDesc));
    }
    void VariantCommon(std::string_view name, EnumVariant&& ev) {
        CheckInit(); 
        std::string n {name};
        EnumDesc& ed = mDesc->AsEnum();
        if (ed.variants.contains(n)) {
            throw std::runtime_error("variant " + n + " was already registered");
        }
        ed.variants.emplace(std::move(n), std::move(ev));        
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
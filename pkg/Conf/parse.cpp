#include "conf/parse.h"

#include "conf/validation.h"

#include "description.h"
#include "parse_error.h"

#include <cassert>
#include <charconv>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>


namespace conf {
namespace {

struct ErrorState {
    std::vector<std::string> path;
    std::vector<Error> errors;

    void Err(std::string message) {
        Error e;
        e.path = path;
        e.message = std::move(message);
        errors.push_back(std::move(e));
    }

    void Push(std::string seg) {
        path.push_back(std::move(seg));
    }

    void Pop() {
        assert(!path.empty());
        path.pop_back();
    }

    bool IsEmpty() const {
        assert(path.empty());
        return errors.empty();
    }
};

struct Guard {
    ErrorState& es;

    Guard(ErrorState& es, std::string seg) : es(es) {
        es.Push(std::move(seg));
    }

    ~Guard() {
        es.Pop();
    }
};

size_t FindKeyValuePairEnd(std::string_view input) {
    size_t balance = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '(') {
            ++balance;
        } else if (input[i] == ')') {
            --balance;
        } else if (input[i] == ',' && balance == 0) {
            return i;
        }
    }
    return input.size();
}

void ParseImpl(ErrorState& es, std::string_view data, Desc& d);

void ParseObj(ErrorState& es, std::string_view data, ObjDesc& t) {
    if (!data.starts_with('(')) {
        es.Err("Object must start with (");
        return;
    }
    if (!data.ends_with(')')) {
        es.Err("Object must end with )");
    }
    assert(data.size() >= 2);
    data = data.substr(1, data.size() - 2);
    if (data.empty()) {
        return;
    }
    std::set<std::string> usedKeys;
    while (!data.empty()) {
        size_t pos = FindKeyValuePairEnd(data);
        std::string_view firstKv = data.substr(0, pos);
        size_t eqPos = firstKv.find('=');
        if (eqPos == std::string_view::npos) {
            es.Err("Key-value pair must contain =");
            return;
        }
        std::string key {firstKv.substr(0, eqPos)};
        Guard g{es, std::string{key}};
        std::string_view value = firstKv.substr(eqPos+1);
        if (!usedKeys.insert(key).second) {
            es.Err("Duplicated field");
        } else if (!t.fields.contains(key)) {
            es.Err("Unknown field");
        } else {
            ParseImpl(es, value, t.fields.at(key));
        }
        data = data.substr(std::min(pos+1, data.size()));
    }
}

void ParseEnum(ErrorState& es, std::string_view data, EnumDesc& t) {
    if (!data.starts_with('[')) {
        es.Err("Enum must start with [");
        return;
    }
    if (!data.ends_with(']')) {
        es.Err("Enum must end with ]");
    }
    assert(data.size() >= 2);
    data = data.substr(1, data.size() - 2);
    if (data.empty()) {
        return;
    }
    size_t sepPos = data.find(':');
    std::string variantName;
    std::string_view value;
    if (sepPos == std::string_view::npos) {
        variantName = data;
        return;
    } else {
        variantName = data.substr(0, sepPos);
        value = data.substr(sepPos+1);
    }
    Guard g{es, variantName};
    if (!t.variants.contains(variantName)) {
        es.Err("Unknown variant");
        return;
    }
    
    EnumVariant& v = t.variants.at(variantName);
    *v.flag = true;
    if (v.content) {
        if (sepPos == std::string_view::npos) {
            es.Err("Non-simple variant requires value, but : was not found");
            return;
        }
        ParseImpl(es, value, *v.content);
    } else if (sepPos != std::string_view::npos) {
        es.Err("Simple variant does not take value, but : was found");
    }
}

void ParseBool(ErrorState& es, std::string_view data, BoolDesc& t) {
    if (data == "true") {
        *t.field = true;
    } else if (data == "false") {
        *t.field = false;
    } else {
        es.Err("Unknown boolean value '" + std::string{data} + "'");
    }
}

void ParseNum(ErrorState& es, std::string_view data, NumDesc& t) {
    std::from_chars_result res = t.parser(data);
    if (res.ptr == data.data() + data.size()) {
        return;
    }
    if (res.ec == std::errc::result_out_of_range) {
        es.Err("Value is out of range for size_t: " + std::string{data});
    } else if (res.ec == std::errc::invalid_argument) {
        es.Err("Value is not integer: " + std::string{data});
    } else if (res.ec == std::errc{}) {
        size_t parsedCnt = res.ptr - data.data();
        std::string_view unparsed = data.substr(parsedCnt);
        es.Err("Value has invalid trailing data: '" + std::string{unparsed} + "'");
    } else {
        es.Err("Parsing error for input'" + std::string{data} + "': " + std::make_error_code(res.ec).message());
    }
}

void ParseImpl(ErrorState& es, std::string_view data, Desc& d) {
    if (d.IsBool()) {
        ParseBool(es, data, d.AsBool());
    } else if (d.IsObj()) {
        ParseObj(es, data, d.AsObj());
    } else if (d.IsEnum()) {
        ParseEnum(es, data, d.AsEnum());
    } else if (d.IsNum()) {
        ParseNum(es, data, d.AsNum());
    } else {
        assert(false);
    }
}

void PostprocessImpl(ErrorState& es, Desc& d);

void InvokePostprocess(ErrorState& es, Target& t) {
    try {
        t.Postprocess();
    } catch (BindingException const& ex) {
        es.Err(ex.what());
    }
}

void PostprocessObj(ErrorState& es, ObjDesc& t) {
    for (auto& [name, field] : t.fields) {
        Guard g {es, std::string{name}};
        PostprocessImpl(es, field);
    }
    InvokePostprocess(es, *t.ref);
}

void PostprocessEnum(ErrorState& es, EnumDesc& t) {
    for (auto& [name, var] : t.variants) {
        if (!*var.flag) {
            continue;
        }
        Guard g {es, std::string{name}};
        PostprocessImpl(es, *var.content);
    }
    InvokePostprocess(es, *t.ref);
}

void PostprocessImpl(ErrorState& es, Desc& d) {
    if (d.IsObj()) {
        PostprocessObj(es, d.AsObj());
    } else if (d.IsEnum()) {
        PostprocessEnum(es, d.AsEnum());
    } else if (d.IsBool() || d.IsNum()) {
    } else {
        std::abort();
    }
}

}

void Parse(std::string_view data, Target& t) {
    Desc d = Describe(t);
    ErrorState es;
    ParseImpl(es, data, d);
    
    if (!es.IsEmpty()) {
        throw MakeException(es.errors);
    }
    PostprocessImpl(es, d);
    if (!es.IsEmpty()) {
        throw MakeException(es.errors);
    }
}
}
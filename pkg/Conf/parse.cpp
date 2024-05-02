#include "conf/conf.h"

#include "description.h"
#include "error.h"

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
        std::string_view first_kv = data.substr(0, pos);
        size_t eq_pos = first_kv.find('=');
        if (eq_pos == std::string_view::npos) {
            es.Err("Key-value pair must contain =");
            return;
        }
        std::string key {first_kv.substr(0, eq_pos)};
        Guard g{es, std::string{key}};
        std::string_view value = first_kv.substr(eq_pos+1);
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
    size_t sep_pos = data.find(':');
    if (sep_pos == std::string_view::npos) {
        es.Err("Key-value pair must contain :");
        return;
    }
    std::string key {data.substr(0, sep_pos)};
    Guard g{es, std::string{key}};
    std::string_view value = data.substr(sep_pos+1);
    if (!t.variants.contains(key)) {
        es.Err("Unknown variant");
    } else {
        EnumVariant& v = t.variants.at(key);
        *v.flag = true;
        ParseImpl(es, value, *v.content);
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

void ParseSizeT(ErrorState& es, std::string_view data, SizeTDesc& t) {
    size_t x;
    char const* last = data.data() + data.size();
    std::from_chars_result res = std::from_chars(data.data(), last, x);
    if (res.ptr == last) {
        *t.field = x;
        return;
    }
    if (res.ec == std::errc::result_out_of_range) {
        es.Err("Value is out of range for size_t: " + std::string{data});
    } else if (res.ec == std::errc::invalid_argument) {
        es.Err("Value is not integer: " + std::string{data});
    } else if (res.ec == std::errc{}) {
        size_t unparsedCnt = last - res.ptr;
        std::string_view unparsed {res.ptr, unparsedCnt};
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
    } else if (d.IsSizeT()) {
        ParseSizeT(es, data, d.AsSizeT());
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
    } else if (d.IsBool() || d.IsSizeT()) {
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
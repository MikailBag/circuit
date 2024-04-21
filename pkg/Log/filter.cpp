#include "filter.h"

#include <optional>
#include <set>
#include <string>


namespace logger {
namespace {
std::optional<std::string_view> Parent(std::string_view m) {
    size_t p = m.find_last_of('.');
    if (p == std::string_view::npos) {
        return {};
    }
    return {m.substr(0, p)};
}

class FilterImpl : public Filter {
    friend class Filter;
public:
    bool Enabled(std::string const& module) override {
        if (disableAllLoggers) {
            return false;
        }
        if (disabledLoggers.contains(module)) {
            return false;
        }
        std::string cur = module;
        while (true) {
            if (disabledTrees.contains(cur)) {
                return false;
            }
            std::optional<std::string_view> next = Parent(cur);
            if (next) {
                cur = *next;
            } else {
                break;
            }
        }
        return true;
    }
private:
    std::set<std::string> disabledLoggers;
    std::set<std::string> disabledTrees;
    bool disableAllLoggers = false;

    void AddDisabledLoggerSpec(std::string_view spec) {
        if (spec.ends_with(".*")) {
            spec = spec.substr(0, spec.size()-2);
            disabledTrees.emplace(spec);
        } else if (spec == "*") {
            disableAllLoggers = true;
        } else {
            disabledLoggers.emplace(spec);
        }
    }
};
}

std::unique_ptr<Filter> Filter::Parse(std::string_view spec){
    FilterImpl impl;
    while (true) {
        size_t pos = spec.find_first_of(',');
        if (pos == std::string_view::npos) {
            if (!spec.empty()) {
                impl.AddDisabledLoggerSpec(spec);
            }
            break;
        }
        impl.AddDisabledLoggerSpec(spec.substr(0, pos));
        spec = spec.substr(pos+1);
    }
    return std::unique_ptr<Filter>{new FilterImpl(std::move(impl))};
}
}
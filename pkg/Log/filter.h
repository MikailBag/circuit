#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace logger {
class Filter {
public:
    virtual bool Enabled(std::string const& module) = 0;
    virtual ~Filter() = default;



    static std::unique_ptr<Filter> Parse(std::string_view spec);
};
}
#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <map>

namespace argparser {
namespace detail {
class Access;
}
class Option {
public:
    Option(Option&&) = default;
    Option& operator=(Option&&) = default;
    Option& Required();
    Option& DefaultValue(std::string_view dfl);
    ~Option();
private:
    struct Impl;
    Option(std::unique_ptr<Impl>&& impl);

    std::unique_ptr<Impl> mImpl;
    friend class detail::Access;
};

class Parser {
public:
    Parser();
    Option& AddOption(std::string_view name);
    std::map<std::string, std::string> ParseArgv(size_t argc, char const* const* argv) const;
    ~Parser();
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
    friend class detail::Access;
};


}
#include "argparser/argparser.h"

#include <cstdlib>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string_view>
#include <optional>

namespace argparser {
namespace {
bool StartsWith(std::string_view s, std::string_view prefix) {
    if (s.size() < prefix.size()) {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (s[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

[[noreturn]] void Die(std::string_view message)  {
    std::cerr << "Error: " << message << std::endl;
    exit(1);
}
}

struct OptionData {
    bool required;
    std::optional<std::string> defaultValue;
};

namespace detail {
class Access {
public:
    static Option MakeOption();
    static std::map<std::string, std::string> Parse(ParserConfig const& config, size_t argc, char const* const* argv);
    static OptionData const& UnpackOpt(Option const& opt);
};
}
using detail::Access;

struct Option::Impl {
    OptionData data;

    void Validate() {
        if (data.required && data.defaultValue.has_value()) {
            throw std::invalid_argument("Required() and DefaultValue() are conflicting");
        }
    }
};

Option::Option(std::unique_ptr<Impl>&& impl)
    : mImpl(std::move(impl)) {
    }

Option& Option::Required() {
    mImpl->data.required = true;
    mImpl->Validate();
    return *this;
}

Option& Option::DefaultValue(std::string_view dfl) {
    mImpl->data.defaultValue = dfl;
    mImpl->Validate();
    return *this;
}

Option::~Option() = default;

class ParserConfig::Impl {
public:
    std::map<std::string, std::string> Parse(size_t argc, char const* const* argv) const {
        std::map<std::string, std::string> res;
        for (size_t i = 1; i < argc; i += 2) {
            std::string_view flag = argv[i];
            if (i+1 == argc) {
                Die("no value for key");
            }
            if (!StartsWith(flag, "--") || flag.size() == 2) {
                Die("invalid option " + std::string(flag));
            }
            auto key = std::string(flag.substr(2));
            if (!mOpts.contains(key)) {
                Die("unknown option " + std::string(key));
            }
            res[key] = argv[i+1];
        }
        for (auto const& [k, opt] : mOpts) {
            if (!res.contains(k)) {
                if (Access::UnpackOpt(opt).required) {
                    Die("missing required option " + k);
                }
                if (Access::UnpackOpt(opt).defaultValue) {
                    res[k] = *Access::UnpackOpt(opt).defaultValue;
                }
            }   
        }
        return res;
    }

    Option& AddOption(std::string_view name) {
        Option opt = Access::MakeOption();//
        auto [it, inserted] = mOpts.insert(std::pair<std::string, Option>{std::string(name), std::move(opt)});
        if (!inserted) {
            throw std::invalid_argument("name already used");
        }
        return it->second;
    }
private:
    std::map<std::string, Option> mOpts;
};

ParserConfig::ParserConfig(): mImpl(std::unique_ptr<Impl>(new Impl)) {}

ParserConfig::~ParserConfig() = default;

Option& ParserConfig::AddOption(std::string_view name) {
    return mImpl->AddOption(name); 
}

Option Access::MakeOption() {
    std::unique_ptr<Option::Impl> impl{new Option::Impl()};
    return Option{std::move(impl)};
}

OptionData const& Access::UnpackOpt(Option const& opt) {
    return opt.mImpl->data;
}

std::map<std::string, std::string> Access::Parse(ParserConfig const& config, size_t argc, char const* const* argv) {
    return config.mImpl->Parse(argc, argv);
}

std::map<std::string, std::string> ParseArgv(ParserConfig const& config, size_t argc, char const* const* argv) {
    return Access::Parse(config, argc, argv);
}
}
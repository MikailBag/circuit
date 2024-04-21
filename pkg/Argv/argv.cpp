#include "argparser/argparser.h"

#include "lex.h"

#include "log/log.h"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string_view>
#include <variant>

static logger::Logger L = logger::Get("argparser");

namespace argparser {
namespace {


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

class Parser::Impl {
public:
    std::map<std::string, std::string> Parse(size_t argc, char const* const* argv) const {
        ParseResult res = ParseImpl(argc, argv);
        if (res.helpRequested) {
            for (auto const& [name, opt] : mOpts) {
                std::cout << "--" << name;
                if (Access::UnpackOpt(opt).required) {
                    std::cout << " (required)";
                } else if (Access::UnpackOpt(opt).defaultValue) {
                    std::cout << " [=" << *Access::UnpackOpt(opt).defaultValue << ']';
                }
                std::cout << '\n';
            }
            std::exit(0);
        }
        return res.opts;
    }

    Option& AddOption(std::string_view name) {
        Option opt = Access::MakeOption();
        auto [it, inserted] = mOpts.insert(std::pair<std::string, Option>{std::string(name), std::move(opt)});
        if (!inserted) {
            throw std::invalid_argument("name already used");
        }
        return it->second;
    }
private:
    struct ParseResult {
        std::map<std::string, std::string> opts;
        bool helpRequested = false;
    };
private:
    ParseResult ParseImpl(size_t argc, char const* const* argv) const {
        L().AttrU64("argcAdjusted", argc-1).Log("Parsing arguments");
        lexer::Lexer lex {argc, argv};
        ParseResult res;
        while (lex) {
            L().Log("Parsing next argument");
            lexer::Token t = lex.Next();
            if (!std::holds_alternative<lexer::OptionToken>(t)) {
                Die("expected option, got value " + std::get<lexer::ValueToken>(t).value);
            }
            auto ot = std::get<lexer::OptionToken>(t);
            if (ot.name == "help") {
                L().Log("detected --help");
                res.helpRequested = true;
                continue;
            }
            if (!mOpts.contains(ot.name)) {
                Die("unknown option " + std::string(ot.name));
            }
            if (!lex) {
                Die("unexpected end of options");
            }
            t = lex.Next();
            if (!std::holds_alternative<lexer::ValueToken>(t)) {
                Die("expected value, got option " + std::get<lexer::OptionToken>(t).name);
            }
            auto vt = std::get<lexer::ValueToken>(t);
            res.opts[ot.name] = vt.value;
        }
        for (auto const& [k, opt] : mOpts) {
            if (!res.opts.contains(k)) {
                if (Access::UnpackOpt(opt).required) {
                    Die("missing required option " + k);
                }
                if (Access::UnpackOpt(opt).defaultValue) {
                    res.opts[k] = *Access::UnpackOpt(opt).defaultValue;
                }
            }
        }
        return res;
    }
private:
    std::map<std::string, Option> mOpts;
};

Parser::Parser(): mImpl(std::unique_ptr<Impl>(new Impl)) {}

std::map<std::string, std::string> Parser::ParseArgv(size_t argc, char const* const* argv) const {
    return mImpl->Parse(argc, argv);
}

Parser::~Parser() = default;

Option& Parser::AddOption(std::string_view name) {
    return mImpl->AddOption(name); 
}

Option Access::MakeOption() {
    std::unique_ptr<Option::Impl> impl{new Option::Impl()};
    return Option{std::move(impl)};
}

OptionData const& Access::UnpackOpt(Option const& opt) {
    return opt.mImpl->data;
}
}
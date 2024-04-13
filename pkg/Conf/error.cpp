#include "conf/conf.h"

#include "error.h"

#include <cassert>

namespace conf {
struct ExceptionData {
    std::vector<Error> errors;
};
class ParseException::Impl {
public:
    explicit Impl(ExceptionData const& data): mWhat(Format(data.errors)) {
    }
    std::string const& What() {
        return mWhat;
    }
private:
    static std::string FormatError(Error const& err) {
        std::string res;
        bool first = true;
        res += '.';
        for (std::string const& p : err.path) {
            if (first) {
                first = false;
            } else {
                res += '.';
            }
            res += p;
        }
        res += ": ";
        res += err.message;
        return res;
    }
    static std::string Format(std::vector<Error> const& errors) {
        std::string res;
        bool first = true;
        for (Error const& err : errors) {
            if (first) {
                first = false;
            } else {
                res += "; ";
            }
            res += FormatError(err);
        } 
        return res;
    }
private:
    std::string mWhat;
};

namespace detail {
class ParseExceptionAccess {
public:
    static ParseException Create(ExceptionData data) {
        return ParseException{std::unique_ptr<ParseException::Impl>{new ParseException::Impl{data}}};
    }
};
}

ParseException::ParseException(std::unique_ptr<ParseException::Impl> impl): mImpl(std::move(impl)) {
}

ParseException MakeException(std::vector<Error> errors) {
    assert(!errors.empty());
    ExceptionData d;
    d.errors = std::move(errors);
    
    ParseException e = detail::ParseExceptionAccess::Create(std::move(d));
    return e;
}

char const* ParseException::what() const noexcept {
    return mImpl->What().c_str();
}

ParseException::~ParseException() = default;
}
#include "lex.h"

#include "log/log.h"


#include <stdexcept>
#include <string_view>


static logger::Logger L = logger::Get("argparser.lex");

namespace argparser::lexer {

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


Token AsToken(std::string_view s) {
    if (StartsWith(s, "--")) {
        OptionToken ot;
        ot.name = s.substr(2);
        return ot;
    }
    ValueToken vt;
    vt.value = s;
    return vt;
}

}

Lexer::Lexer(size_t argc, char const* const* argv) : mArgs(argv), mArgCnt(argc-1), mPos(1) {
    if (argc == 0) {
        throw std::invalid_argument("argc=0");
    }
}

Lexer::operator bool() const {
    return mPos <= mArgCnt;
}

Token Lexer::Next() {
    if (mPos > mArgCnt) {
        throw std::runtime_error("lexer is exhausted");
    }
    std::string_view s = mArgs[mPos++];
    L().AttrU64("pos", mPos-1).AttrU64("total", mArgCnt).AttrS("token", s).Log("Fetching new token");
    return AsToken(s);
}
}
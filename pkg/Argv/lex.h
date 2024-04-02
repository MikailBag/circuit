#include <string>
#include <variant>
#include <vector>


namespace argparser::lexer {
struct OptionToken {
    std::string name;
};

struct ValueToken {
    std::string value;
};

using Token = std::variant<OptionToken, ValueToken>;

class Lexer {
public:
    Lexer(size_t argc, char const* const* argv);

    operator bool() const;

    Token Next();
private:
    char const* const* mArgs;
    size_t mArgCnt;
    size_t mPos;
};
}
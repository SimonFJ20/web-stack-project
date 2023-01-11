#include "error.hpp"
#include "utils/all.hpp"
#include <optional>
#include <string_view>

namespace scriptlang {

enum class Tokens {
    Eof,

    MultilineComment,
    SinglelineComment,

    Id,
    Int,
    Float,
    String,

    If,
    Else,
    For,
    Loop,
    While,
    Break,
    Continue,
    Fn,
    Return,
    False,
    True,
    And,
    Or,
    Xor,

    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    Dot,
    Comma,
    Colon,
    Semicolon,

    Plus,
    DoublePlus,
    PlusEqual,

    Minus,
    ThinArrow,
    DoubleMinus,
    MinusEqual,

    Asterisk,
    AsteriskEqual,

    Slash,
    SlashEqual,

    Percent,
    PercentEqual,

    Power,
    PowerEqual,

    Equal,
    FatArrow,
    DoubleEqual,

    Exclamation,
    ExclamationEqual,

    Less,
    LessEqual,

    Greater,
    GreaterEqual,
};

struct Location {
    int line, column;
};

struct Span {
    Location from, to;
};

struct Token {
    Tokens type;
    size_t index, length;
    Span span;
};

class Lexer {
public:
    Lexer(std::string_view text)
        : text { text }
    { }
    auto next() noexcept -> Result<Token, Errors> { return make_token(); }
    auto peek() noexcept -> Result<Token, Errors>
    {
        if (last_token)
            return Result<Token, Errors>::create_ok(*last_token);
        return Errors::LexerNoTokenYet;
    }

private:
    auto make_token() noexcept -> Result<Token, Errors>;
    auto make_number() noexcept -> Result<Token, Errors>;
    auto make_id() noexcept -> Result<Token, Errors>;
    auto id_or_keyword_type(std::string_view substring) noexcept -> Tokens;
    auto make_string() noexcept -> Result<Token, Errors>;
    auto make_static() noexcept -> Result<Token, Errors>;
    auto static_token_type() noexcept -> Result<Tokens, Errors>;
    auto skip_multiline_comment() noexcept -> Result<Tokens, Errors>;
    auto skip_singleline_comment() noexcept -> Result<Tokens, Errors>;

    [[nodiscard]] auto constexpr inline current_location() const noexcept
        -> Location
    {
        return { line, column };
    }
    [[nodiscard]] auto constexpr inline token(
        Tokens type, size_t begin, Location span_from) noexcept -> Token
    {
        auto token = Token {
            type,
            begin,
            index - begin,
            { span_from, { line, column } },
        };
        last_token = token;
        return token;
    }
    [[nodiscard]] auto constexpr inline done() const noexcept -> bool
    {
        return index >= text.size();
    }
    [[nodiscard]] auto constexpr inline current() const noexcept -> char
    {
        return text.at(index);
    }
    auto constexpr inline step() noexcept -> void
    {
        if (done())
            return;
        index++;
        column++;
        if (!done() and text.at(index) == '\n') {
            column = 1;
            line++;
        }
    }

    std::string_view text;
    size_t index = 0;
    int line = 1;
    int column = 1;
    std::optional<Token> last_token;
};

}

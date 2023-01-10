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

struct Token {
    Tokens type;
    size_t index, length;
    Location location;
};

class Lexer {
public:
    Lexer(std::string_view text)
        : text { text }
    { }
    auto constexpr next() noexcept -> Result<Token, void>;
    auto peek() noexcept -> Result<Token, void>
    {
        if (last_token)
            return Result<Token, void>::create_ok(*last_token);
        return {};
    }

private:
    auto constexpr make_number() noexcept -> Result<Token, void>;
    auto constexpr make_id() noexcept -> Result<Token, void>;
    auto constexpr id_or_keyword_type(std::string_view substring) noexcept
        -> Tokens;
    auto constexpr make_string() noexcept -> Result<Token, void>;
    auto constexpr make_static() noexcept -> Result<Token, void>;
    auto constexpr static_token_type() noexcept -> Result<Tokens, void>;
    auto constexpr skip_multiline_comment() noexcept -> Result<Tokens, void>;
    auto constexpr skip_singleline_comment() noexcept -> Result<Tokens, void>;

    [[nodiscard]] auto constexpr inline token(
        Tokens type, size_t begin) noexcept -> Token
    {
        auto token = Token { type, begin, index - begin, { line, column } };
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

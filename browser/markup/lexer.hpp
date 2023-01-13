#pragma once

#include "utils/all.hpp"
#include <optional>
#include <string_view>

namespace markup {

enum class TokenTypes {
    Eof,
    Whitespace,
    MultilineComment,
    SinglelineComment,

    Name,
    Int,
    Float,
    String,
    Id, // Example = `#my_id`
    Class, // Example = `.my_class`

    True,
    False,
    Null,

    LBrace,
    RBrace,
    Comma,
    Equal,
};

struct Token {
    TokenTypes type;
    size_t index, length;
    int line, column;
};

class Lexer final {
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
    [[nodiscard]] auto constexpr inline token(
        TokenTypes type, size_t begin) noexcept -> Token
    {
        auto token = Token { type, begin, index - begin, line, column };
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

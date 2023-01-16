#pragma once

#include "utils.hpp"
#include <optional>
#include <string_view>

namespace bong {

enum class Tokens {
    Eof,
    Whitespace,
    SingleLineComment,
    MultiLineComment,

    Name,
    Id,
    Class,

    Int,
    Float,
    String,
    Bool,

    LBrace,
    RBrace,
    LBracket,
    RBracket,

    Equal,
    Colon,
    SemiColon,
    Comma,
};

struct Location {
    int line, col;
};

struct Token {
    Tokens type;
    size_t index, length;
    Location location;
};

class Lexer {
public:
    struct Error {
        std::string message;
        size_t index;
        Location location;
    };

    Lexer(std::string_view text)
        : text { text }
    {
        (void)next();
    }

    auto next() noexcept -> Result<Token, Error>
    {
        return current_token = make_token();
    }
    [[nodiscard]] auto peek() const noexcept -> Result<Token, Error>
    {
        return current_token;
    }

private:
    auto make_token() noexcept -> Result<Token, Error>;
    auto make_name() noexcept -> Result<Token, Error>;
    auto make_number() noexcept -> Result<Token, Error>;
    auto make_static() noexcept -> Result<Token, Error>;
    auto make_whitespace() noexcept -> Result<Token, Error>;
    auto make_singleline_comment() noexcept -> Result<Token, Error>;
    auto make_multiline_comment() noexcept -> Result<Token, Error>;
    auto make_string() noexcept -> Result<Token, Error>;
    auto make_id() noexcept -> Result<Token, Error>;
    auto make_class() noexcept -> Result<Token, Error>;

    auto current() const noexcept -> char { return text.at(index); }
    auto done() const noexcept -> bool { return index >= text.size(); }
    auto step() noexcept -> void { index++; }

    Result<Token, Error> current_token {
        Error { "next() not called first", index, location },
    };
    std::string_view text;
    size_t index { 0 };
    Location location { 1, 1 };
};

}

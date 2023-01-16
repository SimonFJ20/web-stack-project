#include "bong.hpp"
#include "utils.hpp"
#include <cctype>

namespace bong {

auto Lexer::make_token() noexcept -> Result<Token, Error>
{
    auto c = current();
    if (std::isspace(c))
        return make_whitespace();
    else if (std::isdigit(c))
        return make_number();
    else if (std::isalpha(c))
        return make_name();
    else
        return make_static();
}

auto Lexer::make_name() noexcept -> Result<Token, Error>
{
    auto begin_index = index;
    auto begin = location;
    while (!done()
        and (std ::isalpha(current()) or std::isdigit(current())
            or current() == '_' or current() == '-')) {
        step();
    }
    return Token { Tokens::Name, begin_index, index - begin_index, begin };
}

auto Lexer::make_number() noexcept -> Result<Token, Error>;

auto Lexer::make_static() noexcept -> Result<Token, Error>;

auto Lexer::make_whitespace() noexcept -> Result<Token, Error>;

auto Lexer::make_singleline_comment() noexcept -> Result<Token, Error>;

auto Lexer::make_multiline_comment() noexcept -> Result<Token, Error>;

auto Lexer::make_string() noexcept -> Result<Token, Error>;

auto Lexer::make_id() noexcept -> Result<Token, Error>;

auto Lexer::make_class() noexcept -> Result<Token, Error>;

}

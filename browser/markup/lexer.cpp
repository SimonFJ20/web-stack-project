#include "lexer.hpp"
#include "utils/all.hpp"
#include <cctype>
#include <string_view>

namespace markup {

auto constexpr Lexer::next() noexcept -> Result<Token, void>
{
    if (done())
        return token(TokenTypes::Eof, index);
}

}

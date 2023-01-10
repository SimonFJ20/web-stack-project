#include "parser.hpp"

namespace scriptlang {

auto Parser::parse_value() noexcept -> Result<std::unique_ptr<Expression>, void>
{
    switch (lexer.peek()->type) {
        case Tokens::Id:
        default:
            return {};
    }
}

}

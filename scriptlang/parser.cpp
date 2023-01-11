#include "parser.hpp"
#include "error.hpp"
#include <cstdlib>
#include <memory>

namespace scriptlang {

auto parse_expression(bool strictly_values) noexcept
    -> Result<std::unique_ptr<Expression>, Errors>
{
    if (strictly_values)
        return parse_expression(true);
    return Errors::NotImplemented;
}

auto Parser::parse_struct(bool strictly_values) noexcept
    -> Result<std::unique_ptr<Expression>, Errors>
{
    auto values = std::map<std::string, std::unique_ptr<Expression>> {};
    auto first_brace = TRY(lexer.peek());
    if (TRY(lexer.peek()).type == Tokens::LBrace) {
        TRY(lexer.next());
        auto last_brace = TRY(lexer.peek());
        if (last_brace.type != Tokens::RBrace)
            return Errors::ParserStructNotTerminated;
        return {
            std::make_unique<Struct>(
                token_span(first_brace, last_brace), std::move(values)),
        };
    }
    return parse_atom();
}

auto Parser::parse_atom() noexcept
    -> Result<std::unique_ptr<Expression>, Errors>
{
    auto token = TRY(lexer.peek());
    switch (token.type) {
        case Tokens::Id:
            return {
                std::make_unique<Id>(token_span(token, token),
                    token_text(lexer.peek()->index, lexer.peek()->length)),
            };
        case Tokens::Int:
            return {
                std::make_unique<Int>(token_span(token, token),
                    std::atol(
                        token_text(lexer.peek()->index, lexer.peek()->length)
                            .c_str())),
            };
        case Tokens::Float:
            return {
                std::make_unique<Float>(token_span(token, token),
                    std::atof(
                        token_text(lexer.peek()->index, lexer.peek()->length)
                            .c_str())),
            };
        case Tokens::False:
            return {
                std::make_unique<Bool>(token_span(token, token), false),
            };
        case Tokens::True:
            return {
                std::make_unique<Bool>(token_span(token, token), true),
            };
        case Tokens::String:
            return {
                std::make_unique<String>(token_span(token, token),
                    *parse_string_value(
                        token_text(lexer.peek()->index, lexer.peek()->length))),
            };
        default:
            return Errors::ParserExhausted;
    }
}

[[nodiscard]] auto Parser::parse_string_value(std::string_view literal) noexcept
    -> Result<std::string, Errors>
{
    if (literal.size() < 2)
        return Errors::ParserMalformedStringLiteral;
    auto value = std::string {};
    auto escaped = false;
    for (const auto c : literal.substr(1, literal.size() - 2)) {
        if (escaped) {
            value.push_back([&] {
                switch (c) {
                    case 'n':
                        return '\n';
                    case 'r':
                        return '\r';
                    case 't':
                        return '\t';
                    case 'v':
                        return '\n';
                    default:
                        return c;
                }
            }());
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else {
            value.push_back(c);
        }
    }
    return value;
}

}

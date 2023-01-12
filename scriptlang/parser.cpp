#include "parser.hpp"
#include "utils/result.hpp"
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace scriptlang {

auto Parser::parse_expression(bool strictly_values) noexcept
    -> Result<std::unique_ptr<Expression>, Error>
{
    if (strictly_values)
        return parse_array(true);
    return Error { { { 0, 0 }, { 0, 0 } }, "not implemented" };
}

auto Parser::parse_array(bool strictly_values) noexcept
    -> Result<std::unique_ptr<Expression>, Error>
{
    auto values = std::vector<std::unique_ptr<Expression>> {};
    auto first_bracket = *lexer.peek();
    if (first_bracket.type == Tokens::LBracket) {
        (void)lexer.next();
        auto value = parse_expression(strictly_values);
        if (!value)
            return value;
        values.emplace_back(std::move(*value));
        while (lexer.peek()->type == Tokens::Comma) {
            (void)lexer.next();
            if (lexer.peek()->type == Tokens::LBracket)
                break;
            auto value2 = parse_expression(strictly_values);
            values.emplace_back(std::move(*value2));
        }
        auto last_bracket = *lexer.peek();
        if (last_bracket.type != Tokens::RBracket)
            return Error {
                last_bracket.span,
                "unterminated array",
            };
        (void)lexer.next().unwrap();
        return {
            std::make_unique<Array>(
                Token::token_span(first_bracket, last_bracket),
                std::move(values)),
        };
    }
    return parse_struct(strictly_values);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto Parser::parse_struct(bool strictly_values) noexcept
    -> Result<std::unique_ptr<Expression>, Error>
{
    auto values = std::map<std::string, std::unique_ptr<Expression>> {};
    auto first_brace = *lexer.peek();
    if (first_brace.type == Tokens::LBrace) {
        auto name = *lexer.next();
        if (name.type != Tokens::LBrace) {
            if (name.type != Tokens::Id)
                return Error {
                    name.span,
                    "unexpected token, expected Id or String",
                };
            if (lexer.next()->type != Tokens::Colon)
                return Error {
                    lexer.peek()->span,
                    "unexpected token, expected ':'",
                };
            if (auto result = lexer.next(); !result)
                return { std::move(result.unwrap_error()) };
            auto value = parse_expression(strictly_values);
            if (!value)
                return value.transform<std::unique_ptr<Expression>>();
            if (values.find(token_text(name)) != values.end())
                return Error {
                    name.span,
                    "multiple definitions of struct field",
                };
            values.insert_or_assign(token_text(name), std::move(*value));
            while (lexer.peek()->type == Tokens::Comma) {
                auto name2 = *lexer.next();
                if (name2.type == Tokens::RBrace)
                    break;
                if (name2.type != Tokens::Id)
                    return Error {
                        name2.span,
                        "unexpected token, expected Id",
                    };
                if (lexer.next()->type != Tokens::Colon)
                    return Error {
                        lexer.peek()->span,
                        "unexpected token, expected ':'",
                    };
                (void)lexer.next();
                auto value2 = parse_expression(strictly_values);
                if (!value2)
                    return value2.transform<std::unique_ptr<Expression>>();
                if (values.find(token_text(name2)) != values.end())
                    return Error {
                        name2.span,
                        "multiple definitions of struct field",
                    };
                values.insert_or_assign(token_text(name2), std::move(*value2));
            }
        }
        auto last_brace = *lexer.peek();
        if (last_brace.type != Tokens::RBrace)
            return Error {
                last_brace.span,
                fmt::format("unterminated struct, expected '}}', got {}",
                    last_brace.type),
            };
        (void)lexer.next().unwrap();
        return {
            std::make_unique<Struct>(
                Token::token_span(first_brace, last_brace), std::move(values)),
        };
    }
    return parse_atom();
}

auto Parser::parse_atom() noexcept -> Result<std::unique_ptr<Expression>, Error>
{
    auto token = *lexer.peek();
    switch (token.type) {
        case Tokens::Id: {
            auto node = std::make_unique<Id>(Token::token_span(token, token),
                token_text(token.index, token.length));
            (void)lexer.next().unwrap();
            return { std::move(node) };
        }
        case Tokens::Int: {
            auto node = std::make_unique<Int>(Token::token_span(token, token),
                std::atol(token_text(token.index, token.length).c_str()));
            (void)lexer.next().unwrap();
            return { std::move(node) };
        }
        case Tokens::Float: {
            auto node = std::make_unique<Float>(Token::token_span(token, token),
                std::atof(token_text(token.index, token.length).c_str()));
            (void)lexer.next().unwrap();
            return { std::move(node) };
        }
        case Tokens::False: {
            auto node = std::make_unique<Bool>(
                Token::token_span(token, token), false);
            (void)lexer.next().unwrap();
            return { std::move(node) };
        }
        case Tokens::True: {
            auto node
                = std::make_unique<Bool>(Token::token_span(token, token), true);
            (void)lexer.next().unwrap();
            return { std::move(node) };
        }
        case Tokens::String: {
            auto node
                = std::make_unique<String>(Token::token_span(token, token),
                    *parse_string_value(token_text(token.index, token.length)));
            (void)lexer.next().unwrap();
            return { std::move(node) };
        }
        default:
            return Error { token.span, "unexpected token, expected value" };
    }
}

[[nodiscard]] auto Parser::parse_string_value(std::string_view literal) noexcept
    -> Result<std::string, utils::result::Error<std::string>>
{
    if (literal.size() < 2)
        return utils::result::Error<std::string> { "malformed string" };
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

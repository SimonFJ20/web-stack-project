#include "bong.hpp"
#include "utils.hpp"
#include <cctype>
#include <fmt/core.h>
#include <map>
#include <memory>
#include <string_view>

namespace bong {

auto token_type_to_string(Tokens type) noexcept -> std::string_view
{
    switch (type) {
        case Tokens::Eof:
            return "Eof";
        case Tokens::SingleLineWhitespace:
            return "SingleLineWhitespace";
        case Tokens::MultiLineWhitespace:
            return "MultiLineWhitespace";
        case Tokens::SingleLineComment:
            return "SingleLineComment";
        case Tokens::MultiLineComment:
            return "MultiLineComment";
        case Tokens::Name:
            return "Name";
        case Tokens::Id:
            return "Id";
        case Tokens::Class:
            return "Class";
        case Tokens::Int:
            return "Int";
        case Tokens::Float:
            return "Float";
        case Tokens::String:
            return "String";
        case Tokens::Null:
            return "Null";
        case Tokens::False:
            return "False";
        case Tokens::True:
            return "True";
        case Tokens::LBrace:
            return "LBrace";
        case Tokens::RBrace:
            return "RBrace";
        case Tokens::LBracket:
            return "LBracket";
        case Tokens::RBracket:
            return "RBracket";
        case Tokens::Equal:
            return "Equal";
        case Tokens::Colon:
            return "Colon";
        case Tokens::SemiColon:
            return "SemiColon";
        case Tokens::Comma:
            return "Comma";
    }
}

auto Token::value() const noexcept -> std::string_view
{
    return text.substr(location.index, length);
}

auto char_to_escaped_string(char c) noexcept -> std::string
{
    switch (c) {
        case '\n':
            return "\\n";
        case '\t':
            return "\\t";
        case '\r':
            return "\\r";
        case '\f':
            return "\\f";
        case '\v':
            return "\\v";
        default:
            return { c };
    }
}

auto escape_string(std::string_view value) noexcept -> std::string
{
    auto result = std::string {};
    for (auto c : value)
        result += char_to_escaped_string(c);
    return result;
}

auto Token::to_string() const noexcept -> std::string
{
    return fmt::format(
        "Token {{ [{}:{}], {}:{}, \t{}, \033[01;32m\"{}\"\033[00m }}",
        location.index, length, location.line, location.col,
        token_type_to_string(type), escape_string(value()));
}

auto Lexer::collect() noexcept -> Result<std::vector<Token>, Error>
{
    auto tokens = std::vector<Token> {};
    while (true) {
        auto token = peek();
        if (!token)
            return token.transform<std::vector<Token>>();
        else if (token->type == Tokens::Eof)
            break;
        else
            tokens.push_back(*token);
        next();
    }
    return tokens;
}

auto Lexer::make_token() noexcept -> Result<Token, Error>
{
    if (done())
        return Token { Tokens::Eof, location(), 0, text };
    auto c = current();
    if (std::isspace(c) != 0)
        return make_whitespace();
    else if (std::isdigit(c) != 0)
        return make_number();
    else if (std::isalpha(c) != 0)
        return make_name();
    else
        return make_static();
}

auto Lexer::make_whitespace() noexcept -> Result<Token, Error>
{
    auto begin = location();
    while (!done() and std::isspace(current()) != 0 and current() != '\n') {
        step();
    }
    if (!done() and current() == '\n') {
        while (!done() and std::isspace(current()) != 0) {
            step();
        }
        return Token {
            Tokens::MultiLineWhitespace,
            begin,
            length_from(begin),
            text,
        };
    } else {
        return Token {
            Tokens::SingleLineWhitespace,
            begin,
            length_from(begin),
            text,
        };
    }
}

auto substring_matches(std::string_view text, size_t index,
    std::string_view literal) noexcept -> bool
{
    return literal.size() == 4
        and text.substr(index, literal.size()).compare(literal) == 0;
}

auto Lexer::make_name() noexcept -> Result<Token, Error>
{
    auto begin = location();
    while (!done()
        and ((std::isalpha(current()) != 0) or (std::isdigit(current()) != 0)
            or current() == '_')) {
        step();
    }
    auto type = [&] {
        if (substring_matches(text, begin.index, "null"))
            return Tokens::Null;
        else if (substring_matches(text, begin.index, "false"))
            return Tokens::False;
        else if (substring_matches(text, begin.index, "true"))
            return Tokens::True;
        else
            return Tokens::Name;
    }();
    return Token { type, begin, length_from(begin), text };
}

auto Lexer::make_number() noexcept -> Result<Token, Error>
{
    auto begin = location();
    while (!done() and (std::isdigit(current()) != 0)) {
        step();
    }
    if (!done() and current() == '.') {
        step();
        if (done() or std::isdigit(current()) == 0) {
            return Error { "expected digits after '.'", location() };
        }
        while (!done() and (std::isdigit(current()) != 0)) {
            step();
        }
        return Token { Tokens::Float, begin, length_from(begin), text };
    } else {
        return Token { Tokens::Int, begin, length_from(begin), text };
    }
}

auto Lexer::make_static() noexcept -> Result<Token, Error>
{
    switch (current()) {
        case '/':
            return make_comment();
        case '"':
            return make_string();
        case '#':
            return make_id();
        case '.':
            return make_class();
        case '{':
            return make_single_char_token(Tokens::LBrace);
        case '}':
            return make_single_char_token(Tokens::RBrace);
        case '[':
            return make_single_char_token(Tokens::LBracket);
        case ']':
            return make_single_char_token(Tokens::RBracket);
        case '=':
            return make_single_char_token(Tokens::Equal);
        case ':':
            return make_single_char_token(Tokens::Colon);
        case ';':
            return make_single_char_token(Tokens::SemiColon);
        case ',':
            return make_single_char_token(Tokens::Comma);
        default:
            return Error { fmt::format("unexpected character '{}'", current()),
                location() };
    }
}

auto Lexer::make_comment() noexcept -> Result<Token, Error>
{
    auto begin = location();
    step();
    if (current() == '/')
        return make_single_line_comment(begin);
    else if (current() == '*')
        return make_multi_line_comment(begin);
    else
        return Error {
            fmt::format("expected '/' or '*', got '{}'", current()),
            location(),
        };
}

auto Lexer::make_multi_line_comment(Location begin) noexcept
    -> Result<Token, Error>
{
    step();
    step();
    while (!done() and text.at(index - 1) != '*' and current() != '/') {
        step();
    }
    if (done()) {
        return Error { "expected \"*/\", got EOF", location() };
    }
    step();
    return Token { Tokens::MultiLineComment, begin, length_from(begin), text };
}

auto Lexer::make_single_line_comment(Location begin) noexcept
    -> Result<Token, Error>
{
    step();
    while (!done() and current() != '\n') {
        step();
    }
    return Token { Tokens::SingleLineComment, begin, length_from(begin), text };
}

auto Lexer::make_string() noexcept -> Result<Token, Error>
{
    auto begin = location();
    step();
    auto escaped = false;
    while (!done() and (escaped || current() != '"')) {
        escaped = !escaped and current() == '\\';
        step();
    }
    if (done()) {
        return Error {
            fmt::format("expected '\"', got Eof"),
            location(),
        };
    } else if (current() != '\"') {
        return Error {
            fmt::format("expected '\"', got '{}'", current()),
            location(),
        };
    }
    step();
    return Token { Tokens::String, begin, length_from(begin), text };
}

auto Lexer::make_id() noexcept -> Result<Token, Error>
{
    auto begin = location();
    step();
    while (!done()
        and ((std::isalpha(current()) != 0) or (std::isdigit(current()) != 0)
            or current() == '_')) {
        step();
    }
    return Token { Tokens::Id, begin, length_from(begin), text };
}

auto Lexer::make_class() noexcept -> Result<Token, Error>
{
    auto begin = location();
    step();
    while (!done()
        and ((std::isalpha(current()) != 0) or (std::isdigit(current()) != 0)
            or current() == '_')) {
        step();
    }
    return Token { Tokens::Class, begin, length_from(begin), text };
}
auto Lexer::make_single_char_token(Tokens type) noexcept -> Result<Token, Error>
{
    auto begin = location();
    step();
    return Token { type, begin, length_from(begin), text };
}

auto Lexer::step() noexcept -> void
{
    index++;
    col++;
    if (!done() and current() == '\n') {
        line++;
        col = 1;
    }
}

auto Parser::parse_top_level() noexcept -> Result<std::unique_ptr<Node>, Error>
{
    if (!lexer.peek())
        return Error { lexer.peek().unwrap_error() };
    else if (lexer.peek()->type == Tokens::Name)
        return parse_element();
    else
        return parse_value();
}

auto Parser::parse_element() noexcept -> Result<std::unique_ptr<Node>, Error>
{
    auto name = *lexer.peek();

    auto ids = Element::Ids {};
    auto classes = Element::Classes {};
    auto properties = Element::Properties {};
    auto values = Element::Values {};

    if (auto result = lexer.next(); !result)
        return Error { result.unwrap_error() };
    return Result<std::unique_ptr<Node>, Error>::create_ok(
        std::make_unique<Element>(
            Element { std::string { name.value() }, {}, {}, {}, {} }));
}

auto remove_first_char(std::string_view value) noexcept -> std::string_view
{
    return value.substr(1, value.size() - 1);
}

auto Parser::parse_single_line_fields(
    Element::Initializer& initializer) noexcept -> Result<void, Error>
{
    if (lexer.peek()->type == Tokens::Id)
        return parse_single_line_fields_starts_id(initializer);
    else if (lexer.peek()->type == Tokens::Class)
        return parse_single_line_fields_starts_class(initializer);
    else
        return parse_single_line_fields_starts_with_property_or_value(
            initializer);
}

auto Parser::parse_single_line_fields_starts_id(
    Element::Initializer& initializer) noexcept -> Result<void, Error>
{
    initializer.ids.push_back(std::string {
        remove_first_char(lexer.peek()->value()),
    });
    if (auto result = lexer.next(); !result)
        return Error { result.unwrap_error() };
    return parse_single_line_fields_tail(initializer);
}

auto Parser::parse_single_line_fields_starts_class(
    Element::Initializer& initializer) noexcept -> Result<void, Error>
{
    initializer.classes.push_back(std::string {
        remove_first_char(lexer.peek()->value()),
    });
    if (auto result = lexer.next(); !result)
        return Error { result.unwrap_error() };
    return parse_single_line_fields_tail(initializer);
}

auto Parser::parse_single_line_fields_starts_with_property_or_value(
    Element::Initializer& initializer) noexcept -> Result<void, Error>
{
    if (auto result = parse_mandatory_same_line_whitespace(); !result)
        return result;
    if (lexer.peek()->type == Tokens::Name) {
        auto key = lexer.peek();
        if (auto result = lexer.next(); !result)
            return Error { result.unwrap_error() };
        if (auto result = parse_optional_whitespace(); !result)
            return result;
        if (lexer.peek()->type != Tokens::Equal
            and lexer.peek()->type != Tokens::Colon)
            return Error {
                fmt::format(
                    "expected '=' or ':', got {}", lexer.peek()->to_string()),
                lexer.peek()->location,
            };
        if (auto result = lexer.next(); !result)
            return Error { result.unwrap_error() };
        if (auto result = parse_optional_same_line_whitespace(); !result)
            return result;
        auto value = parse_single_line_value();
        if (!value)
            return value.transform<void>();
        initializer.properties.insert_or_assign(
            std::string { key->value() }, *value);
        return parse_single_line_fields_tail(initializer);
    } else {
        auto value = parse_single_line_value();
        if (!value)
            return value.transform<void>();
        initializer.values.push_back(*value);
        return parse_single_line_fields_tail(initializer);
    }
}

auto Parser::parse_single_line_fields_tail(
    Element::Initializer& initializer) noexcept -> Result<void, Error>
{ }

}

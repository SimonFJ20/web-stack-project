#include "lexer.hpp"
#include <cctype>
#include <string>
#include <string_view>

namespace scriptlang {

auto Lexer::make_token() noexcept -> Result<Token, Error>
{
    if (done())
        return token(Tokens::Eof, index, current_location());
    if (std::isspace(current()) != 0)
        return skip_whitespace();
    if (std::isdigit(current()) != 0)
        return make_number();
    if (std::isalpha(current()) != 0 or current() == '_')
        return make_id();
    if (current() == '"')
        return make_string();
    return make_static();
}

auto Lexer::skip_whitespace() noexcept -> Result<Token, Error>
{
    while (!done() and std::isspace(current()) != 0)
        step();
    return make_token();
}

auto Lexer::make_number() noexcept -> Result<Token, Error>
{
    auto begin = index;
    auto span_from = current_location();
    while (!done() and std::isdigit(current()) != 0)
        step();
    if (current() == '.') {
        step();
        while (!done() and std::isdigit(current()) != 0)
            step();
        return token(Tokens::Float, begin, span_from);
    }
    return token(Tokens::Int, begin, span_from);
}

auto Lexer::make_id() noexcept -> Result<Token, Error>
{
    auto begin = index;
    auto span_from = current_location();
    while (!done()
        and (std::isalpha(current()) != 0 or std::isdigit(current()) != 0
            or current() == '_'))
        step();
    return token(id_or_keyword_type(text.substr(begin, index - begin)), begin,
        span_from);
}

auto Lexer::id_or_keyword_type(std::string_view substring) noexcept -> Tokens
{
    if (substring.compare("if") == 0)
        return Tokens::If;
    if (substring.compare("else") == 0)
        return Tokens::Else;
    if (substring.compare("for") == 0)
        return Tokens::For;
    if (substring.compare("loop") == 0)
        return Tokens::Loop;
    if (substring.compare("while") == 0)
        return Tokens::While;
    if (substring.compare("break") == 0)
        return Tokens::Break;
    if (substring.compare("continue") == 0)
        return Tokens::Continue;
    if (substring.compare("fn") == 0)
        return Tokens::Fn;
    if (substring.compare("return") == 0)
        return Tokens::Return;
    if (substring.compare("false") == 0)
        return Tokens::False;
    if (substring.compare("true") == 0)
        return Tokens::True;
    if (substring.compare("and") == 0)
        return Tokens::And;
    if (substring.compare("or") == 0)
        return Tokens::Or;
    if (substring.compare("xor") == 0)
        return Tokens::Xor;
    return Tokens::Id;
}

auto Lexer::make_string() noexcept -> Result<Token, Error>
{
    auto begin = index;
    auto span_from = current_location();
    step();
    auto escaped = false;
    while (!done() and (current() != '"' or escaped)) {
        escaped = escaped ? false : current() == '\\';
        step();
    }
    if (current() != '"')
        return Error {
            { span_from, { line, column } },
            "unterminated string",
        };
    step();
    return token(Tokens::String, begin, span_from);
}

auto Lexer::make_static() noexcept -> Result<Token, Error>
{
    auto begin = index;
    auto span_from = current_location();
    auto type = static_token_type();
    if (!type)
        return type.transform<Token>();
    return token(*type, begin, span_from);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto Lexer::static_token_type() noexcept -> Result<Tokens, Error>
{
    using TT = Tokens;
    auto stepped = [&](Tokens v) {
        step();
        return v;
    };

    if (current() == '(')
        return stepped(TT::LParen);
    if (current() == ')')
        return stepped(TT::RParen);
    if (current() == '{')
        return stepped(TT::LBrace);
    if (current() == '}')
        return stepped(TT::RBrace);
    if (current() == '[')
        return stepped(TT::LBracket);
    if (current() == ']')
        return stepped(TT::RBracket);
    if (current() == '.')
        return stepped(TT::Dot);
    if (current() == ',')
        return stepped(TT::Comma);
    if (current() == ':')
        return stepped(TT::Colon);
    if (current() == ';')
        return stepped(TT::Semicolon);
    if (current() == ']')
        return stepped(TT::RBracket);
    if (current() == '+') {
        step();
        if (current() == '+')
            return stepped(TT::DoublePlus);
        if (current() == '=')
            return stepped(TT::PlusEqual);
        return TT::Plus;
    }
    if (current() == '-') {
        step();
        if (current() == '>')
            return stepped(TT::ThinArrow);
        if (current() == '-')
            return stepped(TT::DoubleMinus);
        if (current() == '=')
            return stepped(TT::MinusEqual);
        return TT::Minus;
    }
    if (current() == '*') {
        step();
        if (current() == '=')
            return TT::AsteriskEqual;
        return TT::Asterisk;
    }
    if (current() == '/') {
        step();
        if (current() == '*')
            return skip_multiline_comment();
        if (current() == '/')
            return skip_singleline_comment();
        if (current() == '=')
            return TT::SlashEqual;
        return TT::Slash;
    }
    if (current() == '%') {
        step();
        if (current() == '=')
            return TT::PercentEqual;
        return TT::Percent;
    }
    if (current() == '^') {
        step();
        if (current() == '=')
            return TT::PowerEqual;
        return TT::Power;
    }
    if (current() == '=') {
        step();
        if (current() == '>')
            return stepped(TT::FatArrow);
        if (current() == '=')
            return stepped(TT::DoubleEqual);
        return TT::Equal;
    }
    if (current() == '!') {
        step();
        if (current() == '=')
            return stepped(TT::ExclamationEqual);
        return TT::Exclamation;
    }
    if (current() == '<') {
        step();
        if (current() == '=')
            return stepped(TT::LessEqual);
        return TT::Less;
    }
    if (current() == '>') {
        step();
        if (current() == '=')
            return stepped(TT::GreaterEqual);
        return TT::Greater;
    }
    return Error {
        { { line, column - 1 }, { line, column } },
        "unexpected character",
    };
}

auto Lexer::skip_multiline_comment() noexcept -> Result<Tokens, Error>
{
    step();
    auto last = current();
    step();
    while (!done() and last != '*' and current() != '/')
        step();
    if (last != '*' or current() != '/')
        return Error {
            { { line, column - 1 }, { line, column } },
            "unterminated multiline comment",
        };
    step();
    return Tokens::MultilineComment;
}

auto Lexer::skip_singleline_comment() noexcept -> Result<Tokens, Error>
{
    step();
    while (!done() and current() != '\n')
        step();
    if (current() == '\n')
        step();
    return Tokens::SinglelineComment;
}

}

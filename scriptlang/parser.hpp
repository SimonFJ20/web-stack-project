#pragma once

#include "lexer.hpp"
#include "utils/all.hpp"
#include "utils/result.hpp"
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace scriptlang {

struct Span {
    Location from, to;
};

enum class Expressions {
    Binary,
    Negate,
    Not,
    Index,
    Access,
    Call,
    Operator,

    Id,
    Int,
    Float,
    Bool,
    String,
};

struct Expression {
public:
    Expression() = default;
    Expression(const Expression&) = delete;
    Expression(Expression&&) = delete;
    auto operator=(const Expression&) -> Expression& = delete;
    auto operator=(Expression&&) -> Expression& = delete;
    virtual ~Expression() = default;
    [[nodiscard]] virtual auto expression_type() const noexcept -> Expressions
        = 0;
    [[nodiscard]] virtual auto span() const noexcept -> Span = 0;

private:
};

class Id final : public Expression {
public:
    Id(std::string value)
        : m_value { std::move(value) }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Id;
    }
    [[nodiscard]] auto value() const noexcept { return m_value; }

private:
    std::string m_value;
};
class Int final : public Expression {
public:
    Int(int64_t value)
        : m_value { value }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Int;
    }
    [[nodiscard]] auto value() const noexcept { return m_value; }

private:
    int64_t m_value;
};
class Float final : public Expression {
public:
    Float(double value)
        : m_value { value }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Float;
    }
    [[nodiscard]] auto value() const noexcept { return m_value; }

private:
    double m_value;
};
class Bool final : public Expression {
public:
    Bool(bool value)
        : m_value { value }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Bool;
    }
    [[nodiscard]] auto value() const noexcept { return m_value; }

private:
    bool m_value;
};
class String final : public Expression {
public:
    String(std::string value)
        : m_value { std::move(value) }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::String;
    }
    [[nodiscard]] auto value() const noexcept -> const std::string&
    {
        return m_value;
    }

private:
    std::string m_value;
};

class Parser final {
public:
    Parser(std::string_view text)
        : text { text }
        , lexer(text)
    { }
    auto parse_expression() noexcept
        -> Result<std::unique_ptr<Expression>, void>;
    auto parse_value() noexcept -> Result<std::unique_ptr<Expression>, void>;

private:
    std::string_view text;
    Lexer lexer;
};

}

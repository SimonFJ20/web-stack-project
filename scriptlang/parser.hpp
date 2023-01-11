#pragma once

#include "error.hpp"
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

enum class Expressions {
    Binary,
    Negate,
    Not,
    Index,
    Access,
    Call,
    Operator,

    Struct,
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

class Struct final : public Expression {
public:
    Struct(Span span, std::map<std::string, std::unique_ptr<Expression>> values)
        : m_span { span }
        , m_values { std::move(values) }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Id;
    }
    [[nodiscard]] auto span() const noexcept -> Span override { return m_span; }
    [[nodiscard]] auto values() const noexcept -> auto& { return m_values; }

private:
    Span m_span;
    std::map<std::string, std::unique_ptr<Expression>> m_values;
};

class Id final : public Expression {
public:
    Id(Span span, std::string value)
        : m_span { span }
        , m_value { std::move(value) }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Id;
    }
    [[nodiscard]] auto span() const noexcept -> Span override { return m_span; }
    [[nodiscard]] auto value() const noexcept { return m_value; }

private:
    Span m_span;
    std::string m_value;
};

class Int final : public Expression {
public:
    Int(Span span, int64_t value)
        : m_span { span }
        , m_value { value }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Int;
    }
    [[nodiscard]] auto span() const noexcept -> Span override { return m_span; }
    [[nodiscard]] auto value() const noexcept { return m_value; }

private:
    Span m_span;
    int64_t m_value;
};

class Float final : public Expression {
public:
    Float(Span span, double value)
        : m_span { span }
        , m_value { value }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Float;
    }
    [[nodiscard]] auto value() const noexcept { return m_value; }
    [[nodiscard]] auto span() const noexcept -> Span override { return m_span; }

private:
    Span m_span;
    double m_value;
};

class Bool final : public Expression {
public:
    Bool(Span span, bool value)
        : m_span { span }
        , m_value { value }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Bool;
    }
    [[nodiscard]] auto value() const noexcept { return m_value; }
    [[nodiscard]] auto span() const noexcept -> Span override { return m_span; }

private:
    Span m_span;
    bool m_value;
};

class String final : public Expression {
public:
    String(Span span, std::string value)
        : m_span { span }
        , m_value { std::move(value) }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::String;
    }
    [[nodiscard]] auto value() const noexcept -> const std::string&
    {
        return m_value;
    }
    [[nodiscard]] auto span() const noexcept -> Span override { return m_span; }

private:
    Span m_span;
    std::string m_value;
};

class Parser final {
public:
    Parser(std::string_view text)
        : text { text }
        , lexer(text)
    { }
    auto parse_expression(bool strictly_values) noexcept
        -> Result<std::unique_ptr<Expression>, Errors>;
    auto parse_struct(bool strictly_values) noexcept
        -> Result<std::unique_ptr<Expression>, Errors>;
    auto parse_atom() noexcept -> Result<std::unique_ptr<Expression>, Errors>;

private:
    [[nodiscard]] static auto parse_string_value(
        std::string_view literal) noexcept -> Result<std::string, Errors>;
    [[nodiscard]] auto token_text(size_t index, size_t length) const noexcept
        -> std::string
    {
        return std::string { text.substr(index, length) };
    }
    [[nodiscard]] static auto token_span(Token from, Token to) noexcept -> Span
    {
        return { from.span.from, to.span.to };
    }

    std::string_view text;
    Lexer lexer;
};

}

#pragma once

#include "lexer.hpp"
#include "utils/all.hpp"
#include "utils/result.hpp"
#include <fmt/core.h>
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

    Array,
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
    [[nodiscard]] virtual auto to_string() const noexcept -> std::string = 0;

private:
};

class Array final : public Expression {
public:
    Array(Span span, std::vector<std::unique_ptr<Expression>> values)
        : m_span { span }
        , m_values { std::move(values) }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Array;
    }
    [[nodiscard]] auto span() const noexcept -> Span override { return m_span; }
    [[nodiscard]] auto values() const noexcept -> auto& { return m_values; }
    [[nodiscard]] auto to_string() const noexcept -> std::string override
    {
        auto values_strings = std::string {};
        auto first = true;
        for (const auto& value : m_values) {
            if (!first)
                values_strings.append(", ");
            first = false;
            values_strings.append(value->to_string());
        }
        return fmt::format("Array {{ [ {} ] }}", values_strings);
    };

private:
    Span m_span;
    std::vector<std::unique_ptr<Expression>> m_values;
};

class Struct final : public Expression {
public:
    Struct(Span span, std::map<std::string, std::unique_ptr<Expression>> values)
        : m_span { span }
        , m_values { std::move(values) }
    { }
    [[nodiscard]] auto expression_type() const noexcept -> Expressions override
    {
        return Expressions::Struct;
    }
    [[nodiscard]] auto span() const noexcept -> Span override { return m_span; }
    [[nodiscard]] auto values() const noexcept -> auto& { return m_values; }
    [[nodiscard]] auto to_string() const noexcept -> std::string override
    {
        auto values_strings = std::string {};
        auto first = true;
        for (const auto& [name, value] : m_values) {
            if (!first)
                values_strings.append(", ");
            first = false;
            values_strings.append(value->to_string());
        }
        return fmt::format("Struct {{ [ {} ] }}", values_strings);
    };

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
    [[nodiscard]] auto to_string() const noexcept -> std::string override
    {
        return fmt::format("Id {{ {} }}", m_value);
    }

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
    [[nodiscard]] auto to_string() const noexcept -> std::string override
    {
        return fmt::format("Int {{ {} }}", m_value);
    }

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
    [[nodiscard]] auto to_string() const noexcept -> std::string override
    {
        return fmt::format("Float {{ {} }}", m_value);
    }

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
    [[nodiscard]] auto to_string() const noexcept -> std::string override
    {
        return fmt::format("Bool {{ {} }}", m_value);
    }

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
    [[nodiscard]] auto to_string() const noexcept -> std::string override
    {
        return fmt::format("String {{ \"{}\" }}", m_value);
    }

private:
    Span m_span;
    std::string m_value;
};

class Parser final {
public:
    Parser(std::string_view text)
        : text { text }
        , lexer(text)
    {
        [[maybe_unused]] auto _ = lexer.next();
    }
    auto parse_expression(bool strictly_values) noexcept
        -> Result<std::unique_ptr<Expression>, Error>;
    auto parse_array(bool strictly_values) noexcept
        -> Result<std::unique_ptr<Expression>, Error>;
    auto parse_struct(bool strictly_values) noexcept
        -> Result<std::unique_ptr<Expression>, Error>;
    auto parse_atom() noexcept -> Result<std::unique_ptr<Expression>, Error>;

private:
    [[nodiscard]] static auto parse_string_value(
        std::string_view literal) noexcept
        -> Result<std::string, utils::result::Error<std::string>>;
    [[nodiscard]] auto token_text(size_t index, size_t length) const noexcept
        -> std::string
    {
        return std::string { text.substr(index, length) };
    }
    [[nodiscard]] auto token_text(const Token& token) const noexcept
        -> std::string
    {
        return std::string { text.substr(token.index, token.length) };
    }

    std::string_view text;
    Lexer lexer;
};

}

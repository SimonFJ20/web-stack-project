#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace utils::result {

class UnwrapError : public std::runtime_error {
public:
    UnwrapError()
        : std::runtime_error("failed to unwrap result")
    { }
};

struct StatesValueTypes {
    struct Value { };
    struct Error { };
};

template <typename InnerValue> struct Extracter {
    using Value = void;
    using Error = void;
};

template <typename Value, typename Error> class [[nodiscard]] Result {
    static_assert(std::is_object_v<Value> && std::is_destructible_v<Value>,
        "incompatible Value in Result<Value, Error>");
    static_assert(std::is_object_v<Error> && std::is_destructible_v<Error>,
        "incompatible Error in Result<Value, Error>");

public:
    constexpr Result(Value&& value) noexcept
        : value_or_error(std::forward<Value>(value))
    { }
    constexpr Result(Error&& error) noexcept
        : value_or_error(std::forward<Error>(error))
    { }

    explicit constexpr Result(
        Value&& value, [[maybe_unused]] StatesValueTypes::Value svt) noexcept
        : value_or_error(std::forward<Value>(value))
    { }
    explicit constexpr Result(
        Error&& error, [[maybe_unused]] StatesValueTypes::Error svt) noexcept
        : value_or_error(std::forward<Error>(error))
    { }

    [[nodiscard]] static auto create_ok(Value value) noexcept
    {
        return Result<Value, Error>(
            std::move(value), StatesValueTypes::Value {});
    }
    [[nodiscard]] static auto create_error(Error error) noexcept
    {
        return Result<Value, Error>(
            std::move(error), StatesValueTypes::Error {});
    }

    [[nodiscard]] constexpr auto is_ok() const noexcept -> bool
    {
        return std::holds_alternative<Value>(value_or_error);
    }
    [[nodiscard]] constexpr auto is_error() const noexcept -> bool
    {
        return std::holds_alternative<Error>(value_or_error);
    }

    [[nodiscard]] constexpr auto unwrap() & -> Value&
    {
        if (!is_ok())
            throw UnwrapError();
        return std::get<Value>(value_or_error);
    }
    [[nodiscard]] constexpr auto unwrap() const& -> const Value&
    {
        if (!is_ok())
            throw UnwrapError();
        return std::get<Value>(value_or_error);
    }

    [[nodiscard]] constexpr auto unwrap_error() & -> Error&
    {
        if (!is_error())
            throw UnwrapError();
        return std::get<Error>(value_or_error);
    }
    [[nodiscard]] constexpr auto unwrap_error() const& -> const Error&
    {
        if (!is_error())
            throw UnwrapError();
        return std::get<Error>(value_or_error);
    }

    [[nodiscard]] constexpr auto unwrap() && -> Value&&
    {
        if (!is_ok())
            throw UnwrapError();
        return std::move(std::get<Value>(value_or_error));
    }
    [[nodiscard]] constexpr auto unwrap() const&& -> const Value&&
    {
        if (!is_ok())
            throw UnwrapError();
        return std::move(std::get<Value>(value_or_error));
    }

    [[nodiscard]] constexpr auto unwrap_error() && -> Error&&
    {
        if (!is_error())
            throw UnwrapError();
        return std::move(std::get<Error>(value_or_error));
    }
    [[nodiscard]] constexpr auto unwrap_error() const&& -> const Error&&
    {
        if (!is_error())
            throw UnwrapError();
        return std::move(std::get<Error>(value_or_error));
    }

    [[nodiscard]] constexpr auto ok() noexcept -> std::optional<Value&>
    {
        return is_ok() ? unwrap() : std::nullopt;
    }
    [[nodiscard]] constexpr auto ok() const noexcept
        -> std::optional<const Value&>
    {
        return is_ok() ? unwrap() : std::nullopt;
    }

    [[nodiscard]] constexpr auto error() noexcept -> std::optional<Error&>
    {
        return is_error() ? unwrap_error() : std::nullopt;
    }
    [[nodiscard]] constexpr auto error() const noexcept
        -> std::optional<const Error&>
    {
        return is_error() ? unwrap_error() : std::nullopt;
    }

    [[nodiscard]] constexpr operator bool() const noexcept { return is_ok(); }

    [[nodiscard]] constexpr auto operator->() -> Value* { return &unwrap(); }
    [[nodiscard]] constexpr auto operator->() const -> const Value*
    {
        return &unwrap();
    }

    [[nodiscard]] constexpr auto operator*() -> Value& { return unwrap(); }
    [[nodiscard]] constexpr auto operator*() const -> const Value&
    {
        return unwrap();
    }

    // Transforms `Result<T, E>` into `Result<Y, E>`.
    // Requries result to be an error.
    template <typename NewValue>
    [[nodiscard]] constexpr auto transform() const noexcept
        -> Result<NewValue, Error>
    {
        return Result<NewValue, Error>::create_error(unwrap_error());
    }

    [[nodiscard]] constexpr auto map(auto func) noexcept
    {
        using NewValue = decltype(func(unwrap()));
        return is_ok() ? Result<NewValue, Error>(func(unwrap()))
                       : Result<NewValue, Error>(unwrap_error());
    }

    [[nodiscard]] constexpr auto map(auto func) const noexcept
    {
        using NewValue = decltype(func(unwrap()));
        return is_ok() ? Result<NewValue, Error>(func(unwrap()))
                       : Result<NewValue, Error>(unwrap_error());
    }

    [[nodiscard]] constexpr auto map_error(auto func) noexcept
    {
        using NewError = decltype(func(unwrap_error()));
        return is_error() ? Result<Value, NewError>(func(unwrap_error()))
                          : Result<Value, NewError>(unwrap());
    }

    [[nodiscard]] constexpr auto map_error(auto func) const noexcept
    {
        using NewError = decltype(func(unwrap_error()));
        return is_error() ? Result<Value, NewError>(func(unwrap_error()))
                          : Result<Value, NewError>(unwrap());
    }

    [[nodiscard]] constexpr auto match(auto if_ok, auto if_error) noexcept
        requires std::same_as<decltype(if_ok()), decltype(if_error())>
    {
        return is_ok() ? if_ok(unwrap()) : if_error(unwrap_error());
    }
    [[nodiscard]] constexpr auto match(auto if_ok, auto if_error) const noexcept
        requires std::same_as<decltype(if_ok()), decltype(if_error())>
    {
        return is_ok() ? if_ok(unwrap()) : if_error(unwrap_error());
    }

    [[nodiscard]] constexpr auto flatten() noexcept
        requires std::same_as<Error, typename Extracter<Value>::Error>
    {
        using InnerValue = typename Extracter<Value>::Value;
        return is_ok() ? unwrap() : Result<InnerValue, Error>(unwrap_error());
    }
    [[nodiscard]] constexpr auto flatten() const noexcept
        requires std::same_as<Error, typename Extracter<Value>::Error>
    {
        using InnerValue = typename Extracter<Value>::Value;
        return is_ok() ? unwrap() : Result<InnerValue, Error>(unwrap_error());
    }

    [[nodiscard]] constexpr auto flatten_error() noexcept
        requires std::same_as<Value, typename Extracter<Error>::Value>
    {
        using InnerError = typename Extracter<Error>::Error;
        return is_error() ? unwrap_error()
                          : Result<Value, InnerError>(unwrap());
    }
    [[nodiscard]] constexpr auto flatten_error() const noexcept
        requires std::same_as<Value, typename Extracter<Error>::Value>
    {
        using InnerError = typename Extracter<Error>::Error;
        return is_error() ? unwrap_error()
                          : Result<Value, InnerError>(unwrap());
    }

    [[nodiscard]] constexpr auto flat_map(auto func) noexcept
    {
        return map(func).flatten();
    }

    [[nodiscard]] constexpr auto flat_map(auto func) const noexcept
    {
        return map(func).flatten();
    }

    [[nodiscard]] constexpr auto flat_map_error(auto func) noexcept
    {
        return map_error(func).flatten_error();
    }

    [[nodiscard]] constexpr auto flat_map_error(auto func) const noexcept
    {
        return map_error(func).flatten_error();
    }

private:
    std::variant<Value, Error> value_or_error;
};

template <typename ValueInferer, typename ErrorInferer>
struct Extracter<Result<ValueInferer, ErrorInferer>> {
    using Value = ValueInferer;
    using Error = ErrorInferer;
};

template <typename Value> class [[nodiscard]] Result<Value, void> {
    static_assert(std::is_object_v<Value> && std::is_destructible_v<Value>,
        "incompatible Error in Result<Value, Error>");

public:
    constexpr Result(Value&& value) noexcept
        : maybe_value { std::forward<Value>(value) } {};
    constexpr Result() noexcept
        : maybe_value {}
    { }

    [[nodiscard]] static auto create_ok(Value value) noexcept
    {
        return Result<Value, void>(std::move(value));
    }
    [[nodiscard]] static auto create_error() noexcept
    {
        return Result<Value, void>();
    }

    [[nodiscard]] constexpr auto is_ok() const noexcept
    {
        return maybe_value.has_value();
    }
    [[nodiscard]] constexpr auto is_error() const noexcept
    {
        return !maybe_value.has_value();
    }

    [[nodiscard]] constexpr auto unwrap() & -> Value&
    {
        if (!is_ok())
            throw UnwrapError();
        return *maybe_value;
    }
    [[nodiscard]] constexpr auto unwrap() const& -> const Value&
    {
        if (!is_ok())
            throw UnwrapError();
        return *maybe_value;
    }

    [[nodiscard]] constexpr auto unwrap() && -> Value&&
    {
        if (!is_ok())
            throw UnwrapError();
        return std::move(*maybe_value);
    }
    [[nodiscard]] constexpr auto unwrap() const&& -> const Value&&
    {
        if (!is_ok())
            throw UnwrapError();
        return std::move(*maybe_value);
    }

    [[nodiscard]] constexpr auto ok() noexcept -> std::optional<Value&>
    {
        return is_ok() ? unwrap() : std::nullopt;
    }
    [[nodiscard]] constexpr auto ok() const noexcept
        -> std::optional<const Value&>
    {
        return is_ok() ? unwrap() : std::nullopt;
    }

    [[nodiscard]] constexpr operator bool() const noexcept { return is_ok(); }

    [[nodiscard]] constexpr auto operator->() -> Value* { return &unwrap(); }
    [[nodiscard]] constexpr auto operator->() const -> const Value*
    {
        return &unwrap();
    }

    [[nodiscard]] constexpr auto operator*() -> Value& { return unwrap(); }
    [[nodiscard]] constexpr auto operator*() const -> const Value&
    {
        return unwrap();
    }

    [[nodiscard]] constexpr auto map(auto func) noexcept
    {
        using NewValue = decltype(func(unwrap()));
        return is_ok() ? Result<NewValue, void>(func(unwrap()))
                       : Result<NewValue, void>();
    }

    [[nodiscard]] constexpr auto map(auto func) const noexcept
    {
        using NewValue = decltype(func(unwrap()));
        return is_ok() ? Result<NewValue, void>(func(unwrap()))
                       : Result<NewValue, void>();
    }

    [[nodiscard]] constexpr auto map_error(auto func) noexcept
    {
        using NewError = decltype(func());
        return is_error() ? Result<Value, NewError>(func())
                          : Result<Value, NewError>(unwrap());
    }

    [[nodiscard]] constexpr auto map_error(auto func) const noexcept
    {
        using NewError = decltype(func());
        return is_error() ? Result<Value, NewError>(func())
                          : Result<Value, NewError>(unwrap());
    }

    [[nodiscard]] constexpr auto match(auto if_ok, auto if_error) noexcept
        requires std::same_as<decltype(if_ok()), decltype(if_error())>
    {
        return is_ok() ? if_ok(unwrap()) : if_error();
    }
    [[nodiscard]] constexpr auto match(auto if_ok, auto if_error) const noexcept
        requires std::same_as<decltype(if_ok()), decltype(if_error())>
    {
        return is_ok() ? if_ok(unwrap()) : if_error();
    }

    [[nodiscard]] constexpr auto flatten() noexcept
        requires std::same_as<void, typename Extracter<Value>::Error>
    {
        using InnerValue = typename Extracter<Value>::Value;
        return is_ok() ? unwrap() : Result<InnerValue, void>();
    }
    [[nodiscard]] constexpr auto flatten() const noexcept
        requires std::same_as<void, typename Extracter<Value>::Error>
    {
        using InnerValue = typename Extracter<Value>::Value;
        return is_ok() ? unwrap() : Result<InnerValue, void>();
    }

    [[nodiscard]] constexpr auto flat_map(auto func) noexcept
    {
        return map(func).flatten();
    }

    [[nodiscard]] constexpr auto flat_map(auto func) const noexcept
    {
        return map(func).flatten();
    }

    [[nodiscard]] constexpr auto flat_map_error(auto func) noexcept
    {
        return map_error(func).flatten_error();
    }

    [[nodiscard]] constexpr auto flat_map_error(auto func) const noexcept
    {
        return map_error(func).flatten_error();
    }

private:
    std::optional<Value> maybe_value;
};

template <typename Error> class [[nodiscard]] Result<void, Error> {
    static_assert(std::is_object_v<Error> && std::is_destructible_v<Error>,
        "incompatible Error in Result<Value, Error>");

public:
    constexpr Result(Error&& error) noexcept
        : maybe_error { std::forward<Error>(error) }
    { }
    constexpr Result() noexcept
        : maybe_error {} {};

    [[nodiscard]] static auto create_ok() noexcept
    {
        return Result<void, Error>();
    }
    [[nodiscard]] static auto create_error(Error error) noexcept
    {
        return Result<void, Error>(std::move(error));
    }

    [[nodiscard]] constexpr auto is_ok() const noexcept
    {
        return !maybe_error.has_value();
    }
    [[nodiscard]] constexpr auto is_error() const noexcept
    {
        return maybe_error.has_value();
    }

    [[nodiscard]] constexpr auto unwrap_error() & -> Error&
    {
        if (!is_error())
            throw UnwrapError();
        return *maybe_error;
    }
    [[nodiscard]] constexpr auto unwrap_error() const& -> const Error&
    {
        if (!is_error())
            throw UnwrapError();
        return *maybe_error;
    }

    [[nodiscard]] constexpr auto unwrap_error() && -> Error&&
    {
        if (!is_error())
            throw UnwrapError();
        return std::move(*maybe_error);
    }
    [[nodiscard]] constexpr auto unwrap_error() const&& -> const Error&&
    {
        if (!is_error())
            throw UnwrapError();
        return std::move(*maybe_error);
    }

    [[nodiscard]] constexpr auto error() noexcept -> std::optional<Error&>
    {
        return is_error() ? unwrap_error() : std::nullopt;
    }
    [[nodiscard]] constexpr auto error() const noexcept
        -> std::optional<const Error&>
    {
        return is_error() ? unwrap_error() : std::nullopt;
    }

    [[nodiscard]] constexpr operator bool() const noexcept { return is_ok(); }

    [[nodiscard]] constexpr auto map(auto func) noexcept
    {
        using NewValue = decltype(func());
        return is_ok() ? Result<NewValue, Error>(func())
                       : Result<NewValue, Error>(unwrap_error());
    }

    [[nodiscard]] constexpr auto map(auto func) const noexcept
    {
        using NewValue = decltype(func());
        return is_ok() ? Result<NewValue, Error>(func())
                       : Result<NewValue, Error>(unwrap_error());
    }

    [[nodiscard]] constexpr auto map_error(auto func) noexcept
    {
        using NewError = decltype(func(unwrap_error()));
        return is_error() ? Result<void, NewError>(func(unwrap_error()))
                          : Result<void, NewError>();
    }

    [[nodiscard]] constexpr auto map_error(auto func) const noexcept
    {
        using NewError = decltype(func(unwrap_error()));
        return is_error() ? Result<void, NewError>(func(unwrap_error()))
                          : Result<void, NewError>();
    }

    [[nodiscard]] constexpr auto match(auto if_ok, auto if_error) noexcept
        requires std::same_as<decltype(if_ok()), decltype(if_error())>
    {
        return is_ok() ? if_ok() : if_error(unwrap_error());
    }
    [[nodiscard]] constexpr auto match(auto if_ok, auto if_error) const noexcept
        requires std::same_as<decltype(if_ok()), decltype(if_error())>
    {
        return is_ok() ? if_ok() : if_error(unwrap_error());
    }

    [[nodiscard]] constexpr auto flatten_error() noexcept
        requires std::same_as<void, typename Extracter<Error>::Value>
    {
        using InnerError = typename Extracter<Error>::Error;
        return is_error() ? unwrap_error() : Result<void, InnerError>();
    }
    [[nodiscard]] constexpr auto flatten_error() const noexcept
        requires std::same_as<void, typename Extracter<Error>::Value>
    {
        using InnerError = typename Extracter<Error>::Error;
        return is_error() ? unwrap_error() : Result<void, InnerError>();
    }

    [[nodiscard]] constexpr auto flat_map(auto func) noexcept
    {
        return map(func).flatten();
    }

    [[nodiscard]] constexpr auto flat_map(auto func) const noexcept
    {
        return map(func).flatten();
    }

    [[nodiscard]] constexpr auto flat_map_error(auto func) noexcept
    {
        return map_error(func).flatten_error();
    }

    [[nodiscard]] constexpr auto flat_map_error(auto func) const noexcept
    {
        return map_error(func).flatten_error();
    }

private:
    std::optional<Error> maybe_error;
};

template <> class [[nodiscard]] Result<void, void> {
public:
    enum class States { Ok, Error };

    explicit constexpr Result(States state) noexcept
        : state { state } {};

    [[nodiscard]] static constexpr auto create_ok() noexcept
    {
        return Result<void, void>(States::Ok);
    }
    [[nodiscard]] static constexpr auto create_error() noexcept
    {
        return Result<void, void>(States::Error);
    }

    [[nodiscard]] constexpr auto is_ok() const noexcept
    {
        return state == States::Ok;
    }
    [[nodiscard]] constexpr auto is_error() const noexcept
    {
        return state == States::Error;
    }

    [[nodiscard]] constexpr operator bool() const noexcept { return is_ok(); }

    [[nodiscard]] constexpr auto map(auto func) noexcept
    {
        using NewValue = decltype(func());
        return is_ok() ? Result<NewValue, void>(func())
                       : Result<NewValue, void>();
    }

    [[nodiscard]] constexpr auto map(auto func) const noexcept
    {
        using NewValue = decltype(func());
        return is_ok() ? Result<NewValue, void>(func())
                       : Result<NewValue, void>();
    }

    [[nodiscard]] constexpr auto map_error(auto func) noexcept
    {
        using NewError = decltype(func());
        return is_error() ? Result<void, NewError>(func())
                          : Result<void, NewError>();
    }

    [[nodiscard]] constexpr auto map_error(auto func) const noexcept
    {
        using NewError = decltype(func());
        return is_error() ? Result<void, NewError>(func())
                          : Result<void, NewError>();
    }

    [[nodiscard]] constexpr auto match(auto if_ok, auto if_error) noexcept
        requires std::same_as<decltype(if_ok()), decltype(if_error())>
    {
        return is_ok() ? if_ok() : if_error();
    }
    [[nodiscard]] constexpr auto match(auto if_ok, auto if_error) const noexcept
        requires std::same_as<decltype(if_ok()), decltype(if_error())>
    {
        return is_ok() ? if_ok() : if_error();
    }

    [[nodiscard]] constexpr auto flat_map(auto func) noexcept
    {
        return map(func).flatten();
    }

    [[nodiscard]] constexpr auto flat_map(auto func) const noexcept
    {
        return map(func).flatten();
    }

    [[nodiscard]] constexpr auto flat_map_error(auto func) noexcept
    {
        return map_error(func).flatten_error();
    }

    [[nodiscard]] constexpr auto flat_map_error(auto func) const noexcept
    {
        return map_error(func).flatten_error();
    }

private:
    States state;
};

}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TRY(expr)                                                              \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (result.is_error())                                                 \
            return { std::move(result.unwrap_error()) };                       \
        std::move(result.unwrap());                                            \
    })

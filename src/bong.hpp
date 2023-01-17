#pragma once

#include "src/result.hpp"
#include "utils.hpp"
#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace bong {

enum class Tokens {
    Eof,
    SingleLineWhitespace,
    MultiLineWhitespace,
    SingleLineComment,
    MultiLineComment,

    Name,
    Id,
    Class,

    Int,
    Float,
    String,
    Null,
    False,
    True,

    LBrace,
    RBrace,
    LBracket,
    RBracket,

    Equal,
    Colon,
    SemiColon,
    Comma,
};

auto token_type_to_string(Tokens type) noexcept -> std::string_view;

struct Location {
    size_t index;
    int line, col;
};

struct Token {
    Tokens type;
    Location location;
    size_t length;
    std::string_view text;

    auto value() const noexcept -> std::string_view;
    auto to_string() const noexcept -> std::string;
};

class Lexer {
public:
    struct Error {
        std::string message;
        Location location;
    };

    inline Lexer(std::string_view text)
        : current_token { make_token() }
        , text { text }
    {
        next();
    }

    inline auto next() noexcept -> Result<Token, Error>
    {
        return current_token = make_token();
    }
    [[nodiscard]] inline auto peek() const noexcept -> Result<Token, Error>
    {
        return current_token;
    }
    [[nodiscard]] auto collect() noexcept -> Result<std::vector<Token>, Error>;

private:
    auto make_token() noexcept -> Result<Token, Error>;
    auto make_whitespace() noexcept -> Result<Token, Error>;
    auto make_name() noexcept -> Result<Token, Error>;
    auto make_number() noexcept -> Result<Token, Error>;
    auto make_static() noexcept -> Result<Token, Error>;
    auto make_comment() noexcept -> Result<Token, Error>;
    auto make_single_line_comment(Location begin) noexcept
        -> Result<Token, Error>;
    auto make_multi_line_comment(Location begin) noexcept
        -> Result<Token, Error>;
    auto make_string() noexcept -> Result<Token, Error>;
    auto make_id() noexcept -> Result<Token, Error>;
    auto make_class() noexcept -> Result<Token, Error>;
    auto make_single_char_token(Tokens type) noexcept -> Result<Token, Error>;

    inline auto current() const noexcept -> char { return text.at(index); }
    inline auto done() const noexcept -> bool { return index >= text.size(); }
    auto step() noexcept -> void;

    auto location() const noexcept -> Location { return { index, line, col }; }
    inline auto length_from(Location begin) const noexcept -> size_t
    {
        return index - begin.index;
    }
    inline auto length_from(size_t begin_index) const noexcept -> size_t
    {
        return index - begin_index;
    }

    Result<Token, Error> current_token;
    std::string_view text;
    size_t index { 0 };
    int line { 1 }, col { 1 };
};

enum class Nodes {
    Element,
    Object,
    Array,
    Int,
    Float,
    Bool,
    String,
};

struct Node {
    Node() = default;
    virtual ~Node() = default;
    virtual auto type() const noexcept -> Nodes;
};

struct Element final : public Node {
    using Ids = std::vector<std::string>;
    using Classes = std::vector<std::string>;
    using Properties = std::map<std::string, std::unique_ptr<Node>>;
    using Values = std::vector<std::unique_ptr<Node>>;
    struct Initializer {
        Ids ids;
        Classes classes;
        Properties properties;
        Values values;
    };

    Element(std::string name, std::vector<std::string> ids,
        std::vector<std::string> classes,
        std::map<std::string, std::unique_ptr<Node>> properties,
        std::vector<std::unique_ptr<Node>> values)
        : name { std::move(name) }
        , ids { std::move(ids) }
        , classes { std::move(classes) }
        , properties { std::move(properties) }
        , values { std::move(values) }
    { }

    std::string name;
    Ids ids;
    Classes classes;
    Properties properties;
    Values values;

    auto type() const noexcept -> Nodes override { return Nodes::Element; }
};

struct Object final : public Node {
    std::map<std::string, std::unique_ptr<Node>> properties;

    auto type() const noexcept -> Nodes override { return Nodes::Object; }
};

struct Array final : public Node {
    std::vector<std::unique_ptr<Node>> values;

    auto type() const noexcept -> Nodes override { return Nodes::Array; }
};

struct Int final : public Node {
    int64_t value;

    auto type() const noexcept -> Nodes override { return Nodes::Int; }
};

struct Float final : public Node {
    double value;

    auto type() const noexcept -> Nodes override { return Nodes::Float; }
};

struct String final : public Node {
    std::string value;

    auto type() const noexcept -> Nodes override { return Nodes::String; }
};

struct Bool final : public Node {
    bool value;

    auto type() const noexcept -> Nodes override { return Nodes::Bool; }
};

// class Parser {
// public:
//     struct Error {
//         Error(Lexer::Error error)
//             : message { std::move(error.message) }
//             , location { error.location }
//         { }
//         Error(std::string message, Location location)
//             : message { std::move(message) }
//             , location { location }
//         { }

//         std::string message;
//         Location location;
//     };

//     Parser(Lexer lexer)
//         : lexer { std::move(lexer) }
//     { }

//     auto parse_top_level() noexcept -> Result<std::unique_ptr<Node>, Error>;
//     auto parse_element() noexcept -> Result<std::unique_ptr<Node>, Error>;
//     auto parse_element_body(Element::Initializer& initializer) noexcept
//         -> Result<void, Error>;
//     auto parse_element_fields(Element::Initializer& initializer) noexcept
//         -> Result<void, Error>;
//     auto parse_element_field(Element::Initializer& initializer) noexcept
//         -> Result<void, Error>;
//     auto parse_single_line_fields(Element::Initializer& initializer) noexcept
//         -> Result<void, Error>;
//     auto parse_element_property() noexcept -> Result<void, Error>;
//     auto parse_single_line_value() noexcept -> Result<void, Error>;
//     auto parse_value() noexcept -> Result<std::unique_ptr<Node>, Error>;
//     auto parse_object() noexcept -> Result<void, Error>;
//     auto parse_object_properties() noexcept -> Result<void, Error>;
//     auto parse_object_property() noexcept -> Result<void, Error>;
//     auto parse_array() noexcept -> Result<void, Error>;
//     auto parse_array_values() noexcept -> Result<void, Error>;
//     auto parse_bool() noexcept -> Result<void, Error>;
//     auto parse_mandatory_same_line_whitespace() noexcept -> Result<void,
//     Error>; auto parse_optional_same_line_whitespace() noexcept ->
//     Result<void, Error>; auto parse_mandatory_linebreak() noexcept ->
//     Result<void, Error>; auto parse_single_line_whitespace() noexcept ->
//     Result<void, Error>; auto parse_line_breaker() noexcept -> Result<void,
//     Error>; auto parse_whitespace_and_line_break() noexcept -> Result<void,
//     Error>; auto parse_optional_whitespace() noexcept -> Result<void, Error>;
//     auto parse_mandatory_whitespace() noexcept -> Result<void, Error>;
//     auto parse_singular_whitespace() noexcept -> Result<void, Error>;

// private:
//     Lexer lexer;
// };

}

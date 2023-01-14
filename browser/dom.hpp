#pragma once

#include <memory>
#include <string>
#include <vector>

namespace dom {

enum class Elements {
    Box,
    Text,
};

class Element {
public:
    Element() = default;
    Element(const Element&) = default;
    Element(Element&&) = delete;
    auto operator=(const Element&) -> Element& = default;
    auto operator=(Element&&) -> Element& = default;
    virtual ~Element() = default;

    [[nodiscard]] virtual auto element_type() const noexcept -> Elements = 0;
    [[nodiscard]] virtual auto parent() noexcept -> std::shared_ptr<Element>
        = 0;
};

class Box final : public Element {
public:
    Box() = default;
    [[nodiscard]] auto element_type() const noexcept -> Elements override
    {
        return Elements::Text;
    };
    auto add_child() noexcept { }

private:
    std::vector<std::shared_ptr<Element>> children;
};

class Text final : public Element {
public:
    Text(std::string value)
        : value { std::move(value) }
    { }
    [[nodiscard]] auto element_type() const noexcept -> Elements override
    {
        return Elements::Text;
    };

private:
    std::string value;
};

}

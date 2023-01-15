#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace dom {

template <typename T> using Ref = std::shared_ptr<T>;

struct Position {
    auto operator+=(const Position& other) noexcept -> Position&
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    int x, y;
};

auto operator+(const Position& a, const Position& b) noexcept -> Position
{
    return { a.x + b.x, a.y + b.y };
}

struct Size {
    auto operator+=(const Size& other) noexcept -> Size&
    {
        width += other.width;
        height += other.height;
        return *this;
    }
    friend auto operator+(const Size& a, const Size& b) noexcept -> Size;

    int width, height;
};

auto operator+(const Size& a, const Size& b) noexcept -> Size
{
    return { a.width + b.width, a.height + b.height };
}

struct Color {
    static const auto opaque = uint8_t { 255 };
    static const auto transparent = uint8_t { 0 };

    uint8_t red, green, blue, alpha { opaque };
};

struct Renderer {
    Renderer() = default;
    virtual ~Renderer() = default;
    virtual auto draw_rectangle(
        Position position, Size size, Color color) const noexcept -> void
        = 0;
};

struct Element {
    Element() = default;
    virtual ~Element() = default;
    [[nodiscard]] virtual auto size() const noexcept -> Size = 0;
    virtual auto render(Renderer& renderer, Position position) const noexcept
        -> void
        = 0;
};

struct Box final : public Element {
    [[nodiscard]] auto size() const noexcept -> Size override;
    auto render(Renderer& renderer, Position position) const noexcept
        -> void override;
    auto add_child(Ref<Element> element) noexcept
    {
        children.push_back(element);
    }

    std::vector<Ref<Element>> children;
    Color color {};
};

struct Rectangle final : public Element {
    Rectangle(Size size)
        : m_size { size }
    { }
    [[nodiscard]] auto size() const noexcept -> Size override { return m_size; }
    auto render(Renderer& renderer, Position position) const noexcept
        -> void override;

    Color color {};
    Size m_size { 50, 50 };
};

}

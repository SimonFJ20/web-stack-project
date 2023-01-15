#include "dom.hpp"

namespace dom {

auto Box::size() const noexcept -> Size
{
    auto new_size = Size {};
    for (const auto& child : children)
        new_size += child->size() + Size { 5, 5 };
    new_size += { 5, 5 };
    return new_size;
}

auto Box::render(Renderer& renderer, Position position) const noexcept -> void
{
    renderer.draw_rectangle(position, size(), color);
    auto child_position = position + Position { 5, 5 };
    for (const auto& child : children) {
        child->render(renderer, child_position);
        child_position.y += child->size().height + 5;
    }
}

auto Rectangle::render(Renderer& renderer, Position position) const noexcept
    -> void
{
    renderer.draw_rectangle(position, size(), color);
}

}

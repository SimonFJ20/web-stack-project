#pragma once

class Graphics {
public:
    Graphics(const Graphics&) = default;
    Graphics(Graphics&&) = default;
    auto operator=(const Graphics&) -> Graphics& = default;
    auto operator=(Graphics&&) -> Graphics& = default;
    virtual ~Graphics() = default;

private:
};

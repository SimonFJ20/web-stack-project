#include "SDL_events.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "utils/all.hpp"
#include <SDL.h>
#include <fmt/core.h>
#include <memory>

class GUI {
public:
    GUI(const GUI&) = delete;
    auto operator=(const GUI&) -> GUI& = delete;
    GUI(GUI&&) = delete;
    auto operator=(GUI&&) -> GUI& = delete;
    ~GUI()
    {
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
    };
    friend auto std::make_unique<GUI>(SDL_Window*&, SDL_Renderer*&)
        -> std::unique_ptr<GUI>;
    static auto create() noexcept -> Result<std::unique_ptr<GUI>, void>
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
            return {};
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        if (SDL_CreateWindowAndRenderer(1280, 720, 0, &window, &renderer) != 0)
            return {};
        return std::make_unique<GUI>(window, renderer);
    }
    auto should_exit() noexcept -> bool
    {
        SDL_Event event;
        bool should_exit = false;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT)
                should_exit = true;
        }
        return should_exit;
    }
    auto set_background_color(uint8_t r, uint8_t g, uint8_t b) noexcept -> void
    {
        SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
    }

    auto create_rect(SDL_Rect rect, uint8_t r, uint8_t g, uint8_t b) noexcept
        -> void
    {
        SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &rect);
    }

    auto update_gui() noexcept -> void { SDL_RenderPresent(this->renderer); }

private:
    GUI(SDL_Window* window, SDL_Renderer* renderer)
        : window { window }
        , renderer { renderer }
    { }
    SDL_Window* window;
    SDL_Renderer* renderer;
};

auto main() -> int
{
    // test
    fmt::print("browser: hello world!\n");
    auto gui = GUI::create().unwrap();
    while (true) {
        bool should_exit = gui->should_exit();
        if (should_exit)
            break;
        gui->set_background_color(100, 180, 220);
        SDL_Rect rect = { .x = 0, .y = 0, .w = 50, .h = 50 };
        gui->create_rect(rect, 255, 0, 0);
        gui->update_gui();
    }
}
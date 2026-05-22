#pragma once

#include <SDL.h>

namespace ui {

class ImGuiLayer {
public:
    explicit ImGuiLayer(SDL_Window* window, SDL_GLContext glContext);
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    void handleEvent(const SDL_Event& event);
    void beginFrame();
    void render();
};

} // namespace ui

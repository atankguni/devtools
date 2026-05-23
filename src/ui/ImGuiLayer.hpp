#pragma once

#include "ui/UiSettings.hpp"

#include <SDL.h>

namespace ui {

class ImGuiLayer {
public:
    explicit ImGuiLayer(SDL_Window* window, SDL_GLContext glContext);
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    void handleEvent(const SDL_Event& event);
    void applySettings(const UiSettings& settings);
    void beginFrame();
    void render();

private:
    void loadFont(FontFamily family, float size);

    FontFamily loadedFontFamily_ = FontFamily::System;
    float loadedFontSize_ = 15.0F;
};

} // namespace ui

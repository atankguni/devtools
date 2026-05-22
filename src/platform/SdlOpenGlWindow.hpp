#pragma once

#include <SDL.h>

#include <string>

namespace platform {

class SdlOpenGlWindow {
public:
    SdlOpenGlWindow(std::string title, int width, int height);
    ~SdlOpenGlWindow();

    SdlOpenGlWindow(const SdlOpenGlWindow&) = delete;
    SdlOpenGlWindow& operator=(const SdlOpenGlWindow&) = delete;

    [[nodiscard]] SDL_Window* window() const;
    [[nodiscard]] SDL_GLContext glContext() const;

    void swapBuffers();

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext glContext_ = nullptr;
};

} // namespace platform

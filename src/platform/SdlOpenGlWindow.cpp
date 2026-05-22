#include "platform/SdlOpenGlWindow.hpp"

#include <stdexcept>
#include <utility>

namespace platform {

SdlOpenGlWindow::SdlOpenGlWindow(std::string title, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (window_ == nullptr) {
        const std::string error = SDL_GetError();
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow failed: " + error);
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (glContext_ == nullptr) {
        const std::string error = SDL_GetError();
        SDL_DestroyWindow(std::exchange(window_, nullptr));
        SDL_Quit();
        throw std::runtime_error("SDL_GL_CreateContext failed: " + error);
    }

    SDL_GL_MakeCurrent(window_, glContext_);
    SDL_GL_SetSwapInterval(1);
}

SdlOpenGlWindow::~SdlOpenGlWindow()
{
    if (glContext_ != nullptr) {
        SDL_GL_DeleteContext(glContext_);
    }

    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }

    SDL_Quit();
}

SDL_Window* SdlOpenGlWindow::window() const
{
    return window_;
}

SDL_GLContext SdlOpenGlWindow::glContext() const
{
    return glContext_;
}

void SdlOpenGlWindow::swapBuffers()
{
    SDL_GL_SwapWindow(window_);
}

} // namespace platform

#include "ui/ImGuiLayer.hpp"

#include "ui/Theme.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <filesystem>
#include <optional>
#include <stdexcept>

namespace ui {

namespace {

std::optional<const char*> fontPath(FontFamily family)
{
#if defined(__APPLE__)
    switch (family) {
    case FontFamily::System:
        return "/System/Library/Fonts/SFNS.ttf";
    case FontFamily::Monospace:
        return "/System/Library/Fonts/SFNSMono.ttf";
    case FontFamily::Serif:
        return "/System/Library/Fonts/NewYork.ttf";
    case FontFamily::Classic:
        return std::nullopt;
    }
#elif defined(_WIN32)
    switch (family) {
    case FontFamily::System:
        return "C:/Windows/Fonts/segoeui.ttf";
    case FontFamily::Monospace:
        return "C:/Windows/Fonts/consola.ttf";
    case FontFamily::Serif:
        return "C:/Windows/Fonts/georgia.ttf";
    case FontFamily::Classic:
        return std::nullopt;
    }
#else
    switch (family) {
    case FontFamily::System:
        return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    case FontFamily::Monospace:
        return "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    case FontFamily::Serif:
        return "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf";
    case FontFamily::Classic:
        return std::nullopt;
    }
#endif

    return std::nullopt;
}

} // namespace

ImGuiLayer::ImGuiLayer(SDL_Window* window, SDL_GLContext glContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    loadFont(loadedFontFamily_, loadedFontSize_);

    applyThemeMode(ThemeMode::FollowSystem);

    if (!ImGui_ImplSDL2_InitForOpenGL(window, glContext)) {
        throw std::runtime_error("ImGui SDL2 backend initialization failed");
    }

    if (!ImGui_ImplOpenGL3_Init("#version 150")) {
        ImGui_ImplSDL2_Shutdown();
        throw std::runtime_error("ImGui OpenGL3 backend initialization failed");
    }
}

ImGuiLayer::~ImGuiLayer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::handleEvent(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void ImGuiLayer::applySettings(const UiSettings& settings)
{
    applyThemeMode(settings.themeMode);

    if (settings.fontFamily != loadedFontFamily_ || settings.fontSize != loadedFontSize_) {
        loadFont(settings.fontFamily, settings.fontSize);
    }
}

void ImGuiLayer::loadFont(FontFamily family, float size)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFont* font = nullptr;
    if (family != FontFamily::Classic) {
        const std::optional<const char*> path = fontPath(family);
        if (path.has_value() && std::filesystem::exists(*path)) {
            font = io.Fonts->AddFontFromFileTTF(*path, size);
        }
    }

    if (font == nullptr) {
        font = io.Fonts->AddFontDefault();
    }

    io.FontDefault = font;
    loadedFontFamily_ = family;
    loadedFontSize_ = size;
}

void ImGuiLayer::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace ui

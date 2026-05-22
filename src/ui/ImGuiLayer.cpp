#include "ui/ImGuiLayer.hpp"

#include "ui/Theme.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <filesystem>
#include <stdexcept>

namespace ui {

namespace {

void loadDefaultFont(ImGuiIO& io)
{
#if defined(__APPLE__)
    constexpr const char* fontPath = "/System/Library/Fonts/SFNS.ttf";
    if (std::filesystem::exists(fontPath)
        && io.Fonts->AddFontFromFileTTF(fontPath, 15.0F) != nullptr) {
        return;
    }
#endif

    io.Fonts->AddFontDefault();
}

} // namespace

ImGuiLayer::ImGuiLayer(SDL_Window* window, SDL_GLContext glContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    loadDefaultFont(io);

    applyDarkTheme();

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

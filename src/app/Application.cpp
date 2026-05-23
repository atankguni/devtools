#include "app/Application.hpp"

#include <SDL.h>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <SDL_opengl.h>
#endif

namespace app {

Application::Application()
    : window_("DevTools", 1180, 760)
    , imgui_(window_.window(), window_.glContext())
{
    registerTools();
    shell_.loadSettings(registry_);
}

int Application::run()
{
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            imgui_.handleEvent(event);

            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_WINDOWEVENT
                && event.window.event == SDL_WINDOWEVENT_CLOSE
                && event.window.windowID == SDL_GetWindowID(window_.window())) {
                running = false;
            }
        }

        imgui_.applySettings(shell_.settings());
        imgui_.beginFrame();
        shell_.draw(registry_);

        int drawableWidth = 0;
        int drawableHeight = 0;
        SDL_GL_GetDrawableSize(window_.window(), &drawableWidth, &drawableHeight);

        glViewport(0, 0, drawableWidth, drawableHeight);
        glClearColor(0.08F, 0.09F, 0.10F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        imgui_.render();
        window_.swapBuffers();
    }

    shell_.saveSettings();
    return 0;
}

void Application::registerTools()
{
    registry_.add({
        .id = "json_formatter",
        .name = "JSON Formatter",
        .description = "Format and validate JSON.",
        .draw = [this] { jsonFormatter_.draw(); },
    });

    registry_.add({
        .id = "base64",
        .name = "Base64",
        .description = "Encode and decode Base64 text.",
        .draw = [this] { base64_.draw(); },
    });

    registry_.add({
        .id = "case_converter",
        .name = "Case Converter",
        .description = "Convert text casing.",
        .draw = [this] { caseConverter_.draw(); },
    });

    registry_.add({
        .id = "color",
        .name = "Color Converter",
        .description = "Convert color values.",
        .draw = [this] { color_.draw(); },
    });

    registry_.add({
        .id = "cron",
        .name = "Cron Helper",
        .description = "Preview cron schedules.",
        .draw = [this] { cron_.draw(); },
    });

    registry_.add({
        .id = "hash",
        .name = "Hash Generator",
        .description = "Generate common hashes.",
        .draw = [this] { hash_.draw(); },
    });

    registry_.add({
        .id = "jwt",
        .name = "JWT",
        .description = "Encode and decode JWTs.",
        .draw = [this] { jwt_.draw(); },
    });

    registry_.add({
        .id = "markup",
        .name = "HTML / XML",
        .description = "Format markup text.",
        .draw = [this] { markup_.draw(); },
    });

    registry_.add({
        .id = "password",
        .name = "Password",
        .description = "Generate and check passwords.",
        .draw = [this] { password_.draw(); },
    });

    registry_.add({
        .id = "permissions",
        .name = "Permissions",
        .description = "Calculate chmod values.",
        .draw = [this] { permissions_.draw(); },
    });

    registry_.add({
        .id = "regex",
        .name = "Regex Tester",
        .description = "Test regex patterns.",
        .draw = [this] { regex_.draw(); },
    });

    registry_.add({
        .id = "test_data",
        .name = "Test Data",
        .description = "Generate sample data.",
        .draw = [this] { testData_.draw(); },
    });

    registry_.add({
        .id = "text_diff",
        .name = "Text Diff",
        .description = "Compare two text blocks.",
        .draw = [this] { textDiff_.draw(); },
    });

    registry_.add({
        .id = "url",
        .name = "URL",
        .description = "Encode and decode URLs.",
        .draw = [this] { url_.draw(); },
    });

    registry_.add({
        .id = "uuid",
        .name = "UUID Generator",
        .description = "Generate UUID v4 identifiers.",
        .draw = [this] { uuid_.draw(); },
    });

    registry_.add({
        .id = "timestamp",
        .name = "Timestamp Converter",
        .description = "Convert Unix timestamps.",
        .draw = [this] { timestamp_.draw(); },
    });
}

} // namespace app

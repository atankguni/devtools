cmake_minimum_required(VERSION 3.24)

project(DevTools
    VERSION 0.1.0
    DESCRIPTION "Lightweight native desktop developer utility"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DEVTOOLS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL/CMakeLists.txt")
    message(FATAL_ERROR
        "SDL2 is not vendored yet. Place SDL2 sources in external/SDL before configuring."
    )
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.cpp")
    message(FATAL_ERROR
        "Dear ImGui is not vendored yet. Place Dear ImGui sources in external/imgui before configuring."
    )
endif()

set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)

add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
    external/SDL/include
)

target_compile_features(imgui PUBLIC cxx_std_23)

add_executable(devtools
    src/main.cpp
    src/app/Application.cpp
    src/core/ToolRegistry.cpp
    src/platform/SdlOpenGlWindow.cpp
    src/ui/Clipboard.cpp
    src/ui/ImGuiLayer.cpp
    src/ui/Theme.cpp
    src/ui/UiSettings.cpp
    src/ui/UiShell.cpp
    src/tools/json_formatter/JsonFormatterTool.cpp
    src/tools/base64/Base64Tool.cpp
    src/tools/case_converter/CaseConverterTool.cpp
    src/tools/color/ColorTool.cpp
    src/tools/cron/CronTool.cpp
    src/tools/hash/HashTool.cpp
    src/tools/jwt/JwtTool.cpp
    src/tools/markup/MarkupTool.cpp
    src/tools/number_base/NumberBaseTool.cpp
    src/tools/password/PasswordTool.cpp
    src/tools/permissions/PermissionsTool.cpp
    src/tools/regex/RegexTool.cpp
    src/tools/test_data/TestDataTool.cpp
    src/tools/text_diff/TextDiffTool.cpp
    src/tools/url/UrlTool.cpp
    src/tools/uuid/UuidTool.cpp
    src/tools/timestamp/TimestampTool.cpp
)

target_include_directories(devtools PRIVATE src)
target_compile_features(devtools PRIVATE cxx_std_23)

target_link_libraries(devtools PRIVATE imgui)

if(TARGET SDL2::SDL2-static)
    target_link_libraries(devtools PRIVATE SDL2::SDL2-static)
elseif(TARGET SDL2-static)
    target_link_libraries(devtools PRIVATE SDL2-static)
else()
    target_link_libraries(devtools PRIVATE SDL2)
endif()

if(WIN32)
    target_link_libraries(devtools PRIVATE opengl32)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(devtools PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${OPENGL_LIBRARY}
    )
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(devtools PRIVATE OpenGL::GL)
endif()

if(MSVC)
    target_compile_options(devtools PRIVATE /W4 /permissive-)
    target_compile_options(imgui PRIVATE /W3)
    if(DEVTOOLS_WARNINGS_AS_ERRORS)
        target_compile_options(devtools PRIVATE /WX)
    endif()
else()
    target_compile_options(devtools PRIVATE -Wall -Wextra -Wpedantic)
    if(DEVTOOLS_WARNINGS_AS_ERRORS)
        target_compile_options(devtools PRIVATE -Werror)
    endif()
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES
    src/main.cpp
)

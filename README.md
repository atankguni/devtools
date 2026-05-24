# DevTools

DevTools is a lightweight native desktop developer utility built with C++23,
SDL2, Dear ImGui, and OpenGL. It intentionally avoids web technologies,
embedded browsers, scripting runtimes, package managers, and broad framework
layers.

## Goals

- Native compiled desktop application.
- Fast startup and low memory use.
- Minimal, auditable dependency tree.
- Simple immediate-mode UI.
- Small modules with explicit ownership.
- Easy addition of future tools without a plugin framework.

## Project Layout

```text
project-root/
├── external/
│   ├── imgui/
│   └── SDL/
├── src/
│   ├── app/
│   ├── core/
│   ├── ui/
│   ├── tools/
│   │   ├── base64/
│   │   ├── case_converter/
│   │   ├── color/
│   │   ├── cron/
│   │   ├── hash/
│   │   ├── json_formatter/
│   │   ├── jwt/
│   │   ├── markup/
│   │   ├── number_base/
│   │   ├── password/
│   │   ├── permissions/
│   │   ├── regex/
│   │   ├── sql_formatter/
│   │   ├── test_data/
│   │   ├── text_diff/
│   │   ├── timestamp/
│   │   ├── url/
│   │   └── uuid/
│   ├── platform/
│   └── main.cpp
├── assets/
├── CMakeLists.txt
└── README.md
```

## Dependencies

Vendor these dependencies manually:

- SDL2 source tree in `external/SDL`
- Dear ImGui docking-enabled source tree in `external/imgui`

Recommended setup:

```sh
git clone --branch SDL2 https://github.com/libsdl-org/SDL.git external/SDL
git clone --branch docking https://github.com/ocornut/imgui.git external/imgui
```

The dependency checkouts are intentionally ignored by this repository so local
builds do not add thousands of third-party files to commits.

Expected ImGui backend files:

- `external/imgui/backends/imgui_impl_sdl2.cpp`
- `external/imgui/backends/imgui_impl_opengl3.cpp`

No package manager is required by the project itself. You still need a C++23
compiler, CMake, Git, and normal platform SDK/OpenGL development files.

## Build: macOS

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

```sh
./build/devtools
```

Recommended prerequisites:

```sh
xcode-select --install
brew install cmake git
```

`brew` is optional if you already have CMake and Git installed. The app links
against macOS system frameworks including Cocoa, IOKit, CoreVideo, and OpenGL.

## Build: Linux

Install build tools and OpenGL development files. On Debian or Ubuntu:

```sh
sudo apt update
sudo apt install build-essential cmake git libgl1-mesa-dev
```

Some Linux distributions may also need window-system development headers for
SDL video backends, such as X11 or Wayland packages.

Build and run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/devtools
```

## Build: Windows

Recommended prerequisites:

- Visual Studio 2022 with "Desktop development with C++"
- CMake
- Git

From a Developer PowerShell or normal terminal:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Run the executable:

```text
build/Release/devtools.exe
```

With Ninja or another single-config generator, use the same style as macOS and
Linux:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Build Notes

If the project directory is renamed or moved, an existing CMake cache may still
point to the old path. Remove the ignored `build/` directory and configure
again:

```sh
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

## Implemented Tools

- JSON Formatter: validates and formats JSON using a small local parser.
- Base64: encodes and decodes text.
- Case Converter: converts text to common identifier and title cases.
- Color Converter: converts HEX, RGB, HSL, and HSV values.
- Cron Helper: previews upcoming run times for simple cron expressions.
- CSV / JSON: converts CSV rows and flat JSON object arrays.
- Hash Generator: generates MD5, SHA-1, SHA-256, SHA-384, and SHA-512 digests.
- HTML / XML: formats and minifies markup text.
- JWT: encodes unsigned tokens and decodes JWT header and payload data.
- Line Sort / Dedupe: sorts, trims, filters, and deduplicates line-based text.
- Number Base: converts decimal, octal, hexadecimal, and binary values.
- Password: generates passwords and checks password strength.
- Permissions: converts chmod octal and symbolic permissions.
- Regex Tester: tests regular expressions and captures matches.
- SQL Formatter: beautifies and minifies SQL with configurable indentation.
- String Inspector: reports byte, line, word, control character, and UTF-8 information.
- Test Data: generates local sample names, emails, and JSON objects.
- Text Diff: compares two text blocks with unified diff-style output.
- URL: percent-encodes and decodes URL text.
- URL Parser: breaks URLs into scheme, host, path, query parameters, and fragment.
- UUID Generator: generates UUID v4 values.
- YAML / JSON: converts common YAML maps/lists/scalars and JSON documents.
- Timestamp Converter: converts Unix timestamps to local time.

## Adding a Tool

1. Create a folder under `src/tools/<tool_name>/`.
2. Add a small class with a `draw()` method.
3. Add the new `.cpp` file to `CMakeLists.txt`.
4. Add the tool as a member in `app::Application`.
5. Register it in `Application::registerTools()`.

Example registration:

```cpp
registry_.add({
    .id = "example",
    .name = "Example",
    .description = "Short description.",
    .draw = [this] { example_.draw(); },
});
```

This keeps tool discovery explicit and easy to review. There is no dynamic
plugin loading, service locator, dependency injection container, or global tool
registry.

## Architecture Decisions

The architecture is deliberately direct:

- `platform/` owns SDL and OpenGL window lifetime through RAII.
- `ui/` owns Dear ImGui setup, theme, shell layout, sidebar, and command palette.
- `core/` contains only shared application primitives such as `Tool` and
  `ToolRegistry`.
- `tools/` contains isolated utility modules. Tools do not depend on each other.
- `app/` wires the application together and registers tools explicitly.

The tool system uses composition: each tool is an ordinary object owned by the
application. The registry stores lightweight metadata plus a draw callback. This
is enough for a small desktop utility and avoids framework-like complexity.

The code favors readable C++23 over clever compile-time abstractions. Standard
library facilities are used where they reduce code, but the project avoids
template-heavy patterns, ECS-style organization, dependency injection, and
generic plugin infrastructure.

## Notes

- Docking and keyboard navigation are enabled in Dear ImGui.
- The command palette is opened with `Ctrl+P` or `Cmd+P`.
- Static SDL linking is requested where supported by the vendored SDL CMake
  project.
- The current OpenGL backend targets OpenGL 3.2 core with GLSL `#version 150`,
  which works well across modern Windows, macOS, and Linux desktops.

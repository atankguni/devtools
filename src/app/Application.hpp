#pragma once

#include "core/ToolRegistry.hpp"
#include "platform/SdlOpenGlWindow.hpp"
#include "tools/base64/Base64Tool.hpp"
#include "tools/hash/HashTool.hpp"
#include "tools/json_formatter/JsonFormatterTool.hpp"
#include "tools/jwt/JwtTool.hpp"
#include "tools/password/PasswordTool.hpp"
#include "tools/timestamp/TimestampTool.hpp"
#include "tools/uuid/UuidTool.hpp"
#include "ui/ImGuiLayer.hpp"
#include "ui/UiShell.hpp"

namespace app {

class Application {
public:
    Application();

    int run();

private:
    void registerTools();

    platform::SdlOpenGlWindow window_;
    ui::ImGuiLayer imgui_;
    ui::UiShell shell_;
    core::ToolRegistry registry_;

    tools::json_formatter::JsonFormatterTool jsonFormatter_;
    tools::base64::Base64Tool base64_;
    tools::hash::HashTool hash_;
    tools::jwt::JwtTool jwt_;
    tools::password::PasswordTool password_;
    tools::uuid::UuidTool uuid_;
    tools::timestamp::TimestampTool timestamp_;
};

} // namespace app

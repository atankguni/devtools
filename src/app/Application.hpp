#pragma once

#include "core/ToolRegistry.hpp"
#include "platform/SdlOpenGlWindow.hpp"
#include "tools/base64/Base64Tool.hpp"
#include "tools/case_converter/CaseConverterTool.hpp"
#include "tools/color/ColorTool.hpp"
#include "tools/cron/CronTool.hpp"
#include "tools/hash/HashTool.hpp"
#include "tools/json_formatter/JsonFormatterTool.hpp"
#include "tools/jwt/JwtTool.hpp"
#include "tools/markup/MarkupTool.hpp"
#include "tools/password/PasswordTool.hpp"
#include "tools/permissions/PermissionsTool.hpp"
#include "tools/regex/RegexTool.hpp"
#include "tools/test_data/TestDataTool.hpp"
#include "tools/text_diff/TextDiffTool.hpp"
#include "tools/timestamp/TimestampTool.hpp"
#include "tools/url/UrlTool.hpp"
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
    tools::case_converter::CaseConverterTool caseConverter_;
    tools::color::ColorTool color_;
    tools::cron::CronTool cron_;
    tools::hash::HashTool hash_;
    tools::jwt::JwtTool jwt_;
    tools::markup::MarkupTool markup_;
    tools::password::PasswordTool password_;
    tools::permissions::PermissionsTool permissions_;
    tools::regex_tool::RegexTool regex_;
    tools::test_data::TestDataTool testData_;
    tools::text_diff::TextDiffTool textDiff_;
    tools::url::UrlTool url_;
    tools::uuid::UuidTool uuid_;
    tools::timestamp::TimestampTool timestamp_;
};

} // namespace app

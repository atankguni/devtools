#include "tools/json_formatter/JsonFormatterTool.hpp"

#include "ui/Clipboard.hpp"
#include "ui/Workbench.hpp"

#include <imgui.h>

#include <charconv>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace {

enum class JsonOutputMode {
    TwoSpaces,
    FourSpaces,
    Minified,
};

class JsonFormatter {
public:
    explicit JsonFormatter(std::string_view input, JsonOutputMode outputMode)
        : input_(input)
        , outputMode_(outputMode)
    {
    }

    bool format(std::string& output, std::string& error)
    {
        output_.clear();
        error_.clear();
        skipWhitespace();

        if (!parseValue(0)) {
            error = error_;
            return false;
        }

        skipWhitespace();
        if (position_ != input_.size()) {
            return fail("Unexpected trailing content", error);
        }

        output = output_;
        return true;
    }

private:
    bool fail(std::string_view message)
    {
        error_ = std::string(message) + " at byte " + std::to_string(position_);
        return false;
    }

    bool fail(std::string_view message, std::string& error)
    {
        fail(message);
        error = error_;
        return false;
    }

    void skipWhitespace()
    {
        while (position_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[position_])) != 0) {
            ++position_;
        }
    }

    void writeIndent(int depth)
    {
        output_.append(static_cast<std::size_t>(depth) * indentWidth(), ' ');
    }

    void writeNewline()
    {
        if (!minified()) {
            output_ += '\n';
        }
    }

    void writeSpace()
    {
        if (!minified()) {
            output_ += ' ';
        }
    }

    void writeIndentIfPretty(int depth)
    {
        if (!minified()) {
            writeIndent(depth);
        }
    }

    [[nodiscard]] bool minified() const
    {
        return outputMode_ == JsonOutputMode::Minified;
    }

    [[nodiscard]] unsigned int indentWidth() const
    {
        switch (outputMode_) {
        case JsonOutputMode::TwoSpaces:
            return 2U;
        case JsonOutputMode::FourSpaces:
            return 4U;
        case JsonOutputMode::Minified:
            return 0U;
        }

        return 2U;
    }

    bool consume(char expected)
    {
        if (position_ >= input_.size() || input_[position_] != expected) {
            return false;
        }

        ++position_;
        return true;
    }

    bool parseValue(int depth)
    {
        skipWhitespace();
        if (position_ >= input_.size()) {
            return fail("Expected JSON value");
        }

        switch (input_[position_]) {
        case '{':
            return parseObject(depth);
        case '[':
            return parseArray(depth);
        case '"':
            return parseString();
        case 't':
            return parseLiteral("true");
        case 'f':
            return parseLiteral("false");
        case 'n':
            return parseLiteral("null");
        default:
            if (input_[position_] == '-' || std::isdigit(static_cast<unsigned char>(input_[position_])) != 0) {
                return parseNumber();
            }
            return fail("Expected JSON value");
        }
    }

    bool parseObject(int depth)
    {
        consume('{');
        output_ += '{';
        skipWhitespace();

        if (consume('}')) {
            output_ += '}';
            return true;
        }

        writeNewline();
        while (true) {
            skipWhitespace();
            writeIndentIfPretty(depth + 1);

            if (!parseString()) {
                return false;
            }

            skipWhitespace();
            if (!consume(':')) {
                return fail("Expected ':' after object key");
            }
            output_ += ':';
            writeSpace();

            if (!parseValue(depth + 1)) {
                return false;
            }

            skipWhitespace();
            if (consume('}')) {
                writeNewline();
                writeIndentIfPretty(depth);
                output_ += '}';
                return true;
            }

            if (!consume(',')) {
                return fail("Expected ',' or '}' in object");
            }

            output_ += ',';
            writeNewline();
        }
    }

    bool parseArray(int depth)
    {
        consume('[');
        output_ += '[';
        skipWhitespace();

        if (consume(']')) {
            output_ += ']';
            return true;
        }

        writeNewline();
        while (true) {
            writeIndentIfPretty(depth + 1);

            if (!parseValue(depth + 1)) {
                return false;
            }

            skipWhitespace();
            if (consume(']')) {
                writeNewline();
                writeIndentIfPretty(depth);
                output_ += ']';
                return true;
            }

            if (!consume(',')) {
                return fail("Expected ',' or ']' in array");
            }

            output_ += ',';
            writeNewline();
            skipWhitespace();
        }
    }

    bool parseString()
    {
        if (!consume('"')) {
            return fail("Expected string");
        }

        output_ += '"';
        while (position_ < input_.size()) {
            const char current = input_[position_++];
            output_ += current;

            if (current == '"') {
                return true;
            }

            if (static_cast<unsigned char>(current) < 0x20U) {
                return fail("Control character in string");
            }

            if (current == '\\') {
                if (position_ >= input_.size()) {
                    return fail("Unfinished escape sequence");
                }

                const char escaped = input_[position_++];
                output_ += escaped;
                switch (escaped) {
                case '"':
                case '\\':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                    break;
                case 'u':
                    for (int index = 0; index < 4; ++index) {
                        if (position_ >= input_.size()
                            || std::isxdigit(static_cast<unsigned char>(input_[position_])) == 0) {
                            return fail("Invalid unicode escape");
                        }
                        output_ += input_[position_++];
                    }
                    break;
                default:
                    return fail("Invalid escape sequence");
                }
            }
        }

        return fail("Unterminated string");
    }

    bool parseNumber()
    {
        const std::size_t start = position_;

        if (input_[position_] == '-') {
            ++position_;
        }

        if (position_ >= input_.size()) {
            return fail("Invalid number");
        }

        if (input_[position_] == '0') {
            ++position_;
        } else if (std::isdigit(static_cast<unsigned char>(input_[position_])) != 0) {
            while (position_ < input_.size()
                && std::isdigit(static_cast<unsigned char>(input_[position_])) != 0) {
                ++position_;
            }
        } else {
            return fail("Invalid number");
        }

        if (position_ < input_.size() && input_[position_] == '.') {
            ++position_;
            if (position_ >= input_.size() || std::isdigit(static_cast<unsigned char>(input_[position_])) == 0) {
                return fail("Invalid number fraction");
            }
            while (position_ < input_.size()
                && std::isdigit(static_cast<unsigned char>(input_[position_])) != 0) {
                ++position_;
            }
        }

        if (position_ < input_.size() && (input_[position_] == 'e' || input_[position_] == 'E')) {
            ++position_;
            if (position_ < input_.size() && (input_[position_] == '+' || input_[position_] == '-')) {
                ++position_;
            }
            if (position_ >= input_.size() || std::isdigit(static_cast<unsigned char>(input_[position_])) == 0) {
                return fail("Invalid number exponent");
            }
            while (position_ < input_.size()
                && std::isdigit(static_cast<unsigned char>(input_[position_])) != 0) {
                ++position_;
            }
        }

        output_.append(input_.substr(start, position_ - start));
        return true;
    }

    bool parseLiteral(std::string_view literal)
    {
        if (input_.substr(position_, literal.size()) != literal) {
            return fail("Invalid literal");
        }

        output_.append(literal);
        position_ += literal.size();
        return true;
    }

    std::string_view input_;
    JsonOutputMode outputMode_;
    std::size_t position_ = 0;
    std::string output_;
    std::string error_;
};

} // namespace

namespace tools::json_formatter {

void JsonFormatterTool::draw()
{
    auto runFormatter = [&] {
        JsonOutputMode jsonOutputMode = JsonOutputMode::TwoSpaces;
        switch (outputMode_) {
        case OutputMode::TwoSpaces:
            jsonOutputMode = JsonOutputMode::TwoSpaces;
            break;
        case OutputMode::FourSpaces:
            jsonOutputMode = JsonOutputMode::FourSpaces;
            break;
        case OutputMode::Minified:
            jsonOutputMode = JsonOutputMode::Minified;
            break;
        }

        JsonFormatter formatter(input_.data(), jsonOutputMode);
        if (formatter.format(output_, status_)) {
            status_ = "Valid JSON";
            statusIsError_ = false;
        } else {
            statusIsError_ = true;
        }
    };

    ImGuiIO& io = ImGui::GetIO();
    if ((io.KeyCtrl || io.KeySuper) && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        runFormatter();
    }

    int selectedMode = static_cast<int>(outputMode_);
    constexpr const char* outputModes[] = { "2 spaces", "4 spaces", "Minified" };
    if (ui::workbench::segmentedControl("JsonOutputMode", selectedMode, outputModes, IM_ARRAYSIZE(outputModes))) {
        outputMode_ = static_cast<OutputMode>(selectedMode);
    }

    ImGui::SameLine();
    if (ui::workbench::primaryButton("Format")) {
        runFormatter();
    }

    ImGui::SameLine();
    if (ui::workbench::quietButton("Clear")) {
        input_.fill('\0');
        output_.clear();
        status_.clear();
        statusIsError_ = false;
    }

    ImGui::SameLine();
    if (ui::workbench::quietButton("Copy") && !output_.empty()) {
        ui::copyToClipboard(output_.c_str());
        status_ = "Copied output";
        statusIsError_ = false;
    }

    if (!status_.empty()) {
        ImGui::SameLine();
        ui::workbench::drawStatus(
            status_,
            statusIsError_ ? ui::workbench::StatusTone::Error : ui::workbench::StatusTone::Success
        );
    }

    ImGui::Dummy(ImVec2(0.0F, 6.0F));

    const ImVec2 available = ImGui::GetContentRegionAvail();
    const float gap = 10.0F;
    const bool split = available.x >= 820.0F;
    const ImVec2 paneSize(
        split ? (available.x - gap) * 0.5F : available.x,
        split ? available.y : (available.y - gap) * 0.5F
    );

    if (ui::workbench::beginPanel("JsonInputPane", "Input", "Paste or edit JSON", paneSize)) {
        ImGui::InputTextMultiline(
            "##JsonInput",
            input_.data(),
            input_.size(),
            ImVec2(-1.0F, -1.0F),
            ImGuiInputTextFlags_AllowTabInput
        );
    }
    ui::workbench::endPanel();

    if (split) {
        ImGui::SameLine(0.0F, gap);
    } else {
        ImGui::Dummy(ImVec2(0.0F, gap));
    }

    std::vector<char> outputBuffer(output_.begin(), output_.end());
    outputBuffer.push_back('\0');
    const std::string outputDetail = output_.empty() ? "Result appears here" : std::to_string(output_.size()) + " bytes";
    if (ui::workbench::beginPanel("JsonOutputPane", "Output", outputDetail, ImVec2(0.0F, 0.0F))) {
        ImGui::InputTextMultiline(
            "##JsonOutput",
            outputBuffer.data(),
            outputBuffer.size(),
            ImVec2(-1.0F, -1.0F),
            ImGuiInputTextFlags_ReadOnly
        );
    }
    ui::workbench::endPanel();
}

} // namespace tools::json_formatter

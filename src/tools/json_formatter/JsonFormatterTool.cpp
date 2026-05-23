#include "tools/json_formatter/JsonFormatterTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <charconv>
#include <cctype>
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
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline(
        "##JsonInput",
        input_.data(),
        input_.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 14.0F)
    );

    ImGui::TextUnformatted("Output format");
    int selectedMode = static_cast<int>(outputMode_);
    if (ImGui::RadioButton("2 spaces", selectedMode == static_cast<int>(OutputMode::TwoSpaces))) {
        outputMode_ = OutputMode::TwoSpaces;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("4 spaces", selectedMode == static_cast<int>(OutputMode::FourSpaces))) {
        outputMode_ = OutputMode::FourSpaces;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Minified", selectedMode == static_cast<int>(OutputMode::Minified))) {
        outputMode_ = OutputMode::Minified;
    }

    if (ImGui::Button("Format")) {
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
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        input_.fill('\0');
        output_.clear();
        status_.clear();
    }

    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Output");
    if (!output_.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Copy Output")) {
            ui::copyToClipboard(output_.c_str());
            status_ = "Copied output";
        }
    }

    std::vector<char> outputBuffer(output_.begin(), output_.end());
    outputBuffer.push_back('\0');
    ImGui::InputTextMultiline(
        "##JsonOutput",
        outputBuffer.data(),
        outputBuffer.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 16.0F),
        ImGuiInputTextFlags_ReadOnly
    );
}

} // namespace tools::json_formatter

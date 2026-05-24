#include "tools/yaml_json/YamlJsonTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <cctype>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

struct Node {
    enum class Type {
        String,
        Raw,
        Object,
        Array,
    };

    Type type = Type::String;
    std::string value;
    std::vector<std::pair<std::string, Node>> object;
    std::vector<Node> array;
};

std::string trim(std::string_view value)
{
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1U])) != 0) {
        --end;
    }
    return std::string(value.substr(start, end - start));
}

std::string jsonEscape(std::string_view input)
{
    std::string output;
    for (unsigned char ch : input) {
        switch (ch) {
        case '"':
            output += "\\\"";
            break;
        case '\\':
            output += "\\\\";
            break;
        case '\n':
            output += "\\n";
            break;
        case '\r':
            output += "\\r";
            break;
        case '\t':
            output += "\\t";
            break;
        default:
            output += static_cast<char>(ch);
            break;
        }
    }
    return output;
}

bool isNumber(std::string_view value)
{
    if (value.empty()) {
        return false;
    }
    std::size_t index = value.front() == '-' ? 1U : 0U;
    bool digit = false;
    for (; index < value.size(); ++index) {
        if (std::isdigit(static_cast<unsigned char>(value[index])) != 0) {
            digit = true;
            continue;
        }
        if (value[index] == '.') {
            continue;
        }
        return false;
    }
    return digit;
}

Node scalarNode(std::string value)
{
    value = trim(value);
    if ((value.size() >= 2U && value.front() == '"' && value.back() == '"')
        || (value.size() >= 2U && value.front() == '\'' && value.back() == '\'')) {
        return {.type = Node::Type::String, .value = value.substr(1U, value.size() - 2U)};
    }
    if (value == "true" || value == "false" || value == "null" || isNumber(value)) {
        return {.type = Node::Type::Raw, .value = value};
    }
    return {.type = Node::Type::String, .value = value};
}

std::string renderJson(const Node& node, int depth = 0)
{
    const std::string indent(static_cast<std::size_t>(depth) * 2U, ' ');
    const std::string childIndent(static_cast<std::size_t>(depth + 1) * 2U, ' ');
    std::ostringstream stream;

    switch (node.type) {
    case Node::Type::String:
        stream << '"' << jsonEscape(node.value) << '"';
        break;
    case Node::Type::Raw:
        stream << node.value;
        break;
    case Node::Type::Object:
        stream << "{\n";
        for (std::size_t index = 0; index < node.object.size(); ++index) {
            if (index > 0) {
                stream << ",\n";
            }
            stream << childIndent << '"' << jsonEscape(node.object[index].first) << "\": " << renderJson(node.object[index].second, depth + 1);
        }
        stream << '\n' << indent << '}';
        break;
    case Node::Type::Array:
        stream << "[\n";
        for (std::size_t index = 0; index < node.array.size(); ++index) {
            if (index > 0) {
                stream << ",\n";
            }
            stream << childIndent << renderJson(node.array[index], depth + 1);
        }
        stream << '\n' << indent << ']';
        break;
    }

    return stream.str();
}

struct YamlLine {
    int indent = 0;
    std::string text;
};

std::vector<YamlLine> yamlLines(std::string_view input)
{
    std::vector<YamlLine> lines;
    std::string current;
    auto flush = [&] {
        std::string_view view(current);
        const std::string clean = trim(view);
        if (!clean.empty() && !clean.starts_with('#')) {
            int indent = 0;
            while (indent < static_cast<int>(current.size()) && current[static_cast<std::size_t>(indent)] == ' ') {
                ++indent;
            }
            lines.push_back({.indent = indent, .text = clean});
        }
        current.clear();
    };

    for (char ch : input) {
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            flush();
        } else {
            current += ch;
        }
    }
    flush();
    return lines;
}

Node parseYamlBlock(const std::vector<YamlLine>& lines, std::size_t& index, int indent);

Node parseYamlValue(std::string_view value, const std::vector<YamlLine>& lines, std::size_t& index, int parentIndent)
{
    const std::string clean = trim(value);
    if (!clean.empty()) {
        const std::size_t colon = clean.find(':');
        if (colon != std::string::npos && (colon + 1U == clean.size() || std::isspace(static_cast<unsigned char>(clean[colon + 1U])) != 0)) {
            Node object {.type = Node::Type::Object};
            object.object.push_back({
                trim(std::string_view(clean).substr(0, colon)),
                scalarNode(trim(std::string_view(clean).substr(colon + 1U))),
            });
            if (index < lines.size() && lines[index].indent > parentIndent) {
                Node child = parseYamlBlock(lines, index, lines[index].indent);
                if (child.type == Node::Type::Object) {
                    object.object.insert(object.object.end(), child.object.begin(), child.object.end());
                }
            }
            return object;
        }
        return scalarNode(clean);
    }
    if (index < lines.size() && lines[index].indent > parentIndent) {
        return parseYamlBlock(lines, index, lines[index].indent);
    }
    return {.type = Node::Type::Raw, .value = "null"};
}

Node parseYamlBlock(const std::vector<YamlLine>& lines, std::size_t& index, int indent)
{
    if (index >= lines.size()) {
        return {};
    }

    if (lines[index].text.starts_with("- ")) {
        Node array {.type = Node::Type::Array};
        while (index < lines.size() && lines[index].indent == indent && lines[index].text.starts_with("- ")) {
            const std::string item = trim(std::string_view(lines[index].text).substr(2U));
            ++index;
            array.array.push_back(parseYamlValue(item, lines, index, indent));
        }
        return array;
    }

    Node object {.type = Node::Type::Object};
    while (index < lines.size() && lines[index].indent == indent && !lines[index].text.starts_with("- ")) {
        const std::string text = lines[index].text;
        const std::size_t colon = text.find(':');
        if (colon == std::string::npos) {
            object.object.push_back({text, {.type = Node::Type::Raw, .value = "null"}});
            ++index;
            continue;
        }
        const std::string key = trim(std::string_view(text).substr(0, colon));
        const std::string value = trim(std::string_view(text).substr(colon + 1U));
        ++index;
        object.object.push_back({key, parseYamlValue(value, lines, index, indent)});
    }
    return object;
}

class JsonParser {
public:
    explicit JsonParser(std::string_view input)
        : input_(input)
    {
    }

    bool parse(Node& node, std::string& error)
    {
        if (!parseValue(node, error)) {
            return false;
        }
        skipWhitespace();
        if (position_ != input_.size()) {
            return fail("Unexpected trailing content", error);
        }
        return true;
    }

private:
    void skipWhitespace()
    {
        while (position_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[position_])) != 0) {
            ++position_;
        }
    }

    bool fail(std::string_view message, std::string& error)
    {
        error = std::string(message) + " at byte " + std::to_string(position_);
        return false;
    }

    bool consume(char expected)
    {
        skipWhitespace();
        if (position_ >= input_.size() || input_[position_] != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    bool parseValue(Node& node, std::string& error)
    {
        skipWhitespace();
        if (position_ >= input_.size()) {
            return fail("Expected JSON value", error);
        }
        if (input_[position_] == '"') {
            node.type = Node::Type::String;
            return parseString(node.value, error);
        }
        if (input_[position_] == '{') {
            return parseObject(node, error);
        }
        if (input_[position_] == '[') {
            return parseArray(node, error);
        }

        const std::size_t start = position_;
        while (position_ < input_.size() && input_[position_] != ',' && input_[position_] != '}'
               && input_[position_] != ']' && std::isspace(static_cast<unsigned char>(input_[position_])) == 0) {
            ++position_;
        }
        node.type = Node::Type::Raw;
        node.value = std::string(input_.substr(start, position_ - start));
        return !node.value.empty() || fail("Expected JSON value", error);
    }

    bool parseObject(Node& node, std::string& error)
    {
        node = {.type = Node::Type::Object};
        consume('{');
        if (consume('}')) {
            return true;
        }
        while (true) {
            std::string key;
            Node value;
            if (!parseString(key, error)) {
                return false;
            }
            if (!consume(':')) {
                return fail("Expected ':'", error);
            }
            if (!parseValue(value, error)) {
                return false;
            }
            node.object.push_back({key, std::move(value)});
            if (consume('}')) {
                return true;
            }
            if (!consume(',')) {
                return fail("Expected ',' or '}'", error);
            }
        }
    }

    bool parseArray(Node& node, std::string& error)
    {
        node = {.type = Node::Type::Array};
        consume('[');
        if (consume(']')) {
            return true;
        }
        while (true) {
            Node value;
            if (!parseValue(value, error)) {
                return false;
            }
            node.array.push_back(std::move(value));
            if (consume(']')) {
                return true;
            }
            if (!consume(',')) {
                return fail("Expected ',' or ']'", error);
            }
        }
    }

    bool parseString(std::string& value, std::string& error)
    {
        if (!consume('"')) {
            return fail("Expected string", error);
        }
        value.clear();
        while (position_ < input_.size()) {
            const char ch = input_[position_++];
            if (ch == '"') {
                return true;
            }
            if (ch != '\\') {
                value += ch;
                continue;
            }
            if (position_ >= input_.size()) {
                return fail("Unfinished escape", error);
            }
            const char escaped = input_[position_++];
            switch (escaped) {
            case '"':
            case '\\':
            case '/':
                value += escaped;
                break;
            case 'b':
                value += '\b';
                break;
            case 'f':
                value += '\f';
                break;
            case 'n':
                value += '\n';
                break;
            case 'r':
                value += '\r';
                break;
            case 't':
                value += '\t';
                break;
            default:
                return fail("Unsupported escape", error);
            }
        }
        return fail("Unterminated string", error);
    }

    std::string_view input_;
    std::size_t position_ = 0;
};

bool plainYamlScalar(std::string_view value)
{
    if (value.empty()) {
        return false;
    }
    return value.find_first_of(":#[]{}\",'\n\r\t") == std::string_view::npos;
}

std::string renderYamlScalar(const Node& node)
{
    if (node.type == Node::Type::Raw) {
        return node.value;
    }
    if (plainYamlScalar(node.value)) {
        return node.value;
    }
    return '"' + jsonEscape(node.value) + '"';
}

void renderYaml(const Node& node, std::ostringstream& stream, int depth = 0)
{
    const std::string indent(static_cast<std::size_t>(depth) * 2U, ' ');
    if (node.type == Node::Type::Object) {
        for (const auto& [key, value] : node.object) {
            stream << indent << key << ':';
            if (value.type == Node::Type::Object || value.type == Node::Type::Array) {
                stream << '\n';
                renderYaml(value, stream, depth + 1);
            } else {
                stream << ' ' << renderYamlScalar(value) << '\n';
            }
        }
    } else if (node.type == Node::Type::Array) {
        for (const Node& value : node.array) {
            stream << indent << "-";
            if (value.type == Node::Type::Object || value.type == Node::Type::Array) {
                stream << '\n';
                renderYaml(value, stream, depth + 1);
            } else {
                stream << ' ' << renderYamlScalar(value) << '\n';
            }
        }
    } else {
        stream << indent << renderYamlScalar(node) << '\n';
    }
}

} // namespace

namespace tools::yaml_json {

void YamlJsonTool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline("##YamlJsonInput", input_.data(), input_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 12.0F));

    if (ImGui::Button("YAML to JSON")) {
        yamlToJson();
    }
    ImGui::SameLine();
    if (ImGui::Button("JSON to YAML")) {
        jsonToYaml();
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
    std::vector<char> buffer(output_.begin(), output_.end());
    buffer.push_back('\0');
    ImGui::InputTextMultiline("##YamlJsonOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 16.0F), ImGuiInputTextFlags_ReadOnly);
}

void YamlJsonTool::yamlToJson()
{
    const std::vector<YamlLine> lines = yamlLines(input_.data());
    if (lines.empty()) {
        output_.clear();
        status_ = "No YAML content";
        return;
    }
    std::size_t index = 0;
    const Node root = parseYamlBlock(lines, index, lines.front().indent);
    output_ = renderJson(root);
    status_ = "Converted YAML";
}

void YamlJsonTool::jsonToYaml()
{
    Node root;
    std::string error;
    JsonParser parser(input_.data());
    if (!parser.parse(root, error)) {
        output_.clear();
        status_ = error;
        return;
    }

    std::ostringstream stream;
    renderYaml(root, stream);
    output_ = stream.str();
    status_ = "Converted JSON";
}

} // namespace tools::yaml_json

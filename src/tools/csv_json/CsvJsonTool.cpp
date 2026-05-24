#include "tools/csv_json/CsvJsonTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

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
            if (ch < 0x20U) {
                output += ' ';
            } else {
                output += static_cast<char>(ch);
            }
            break;
        }
    }
    return output;
}

std::vector<std::vector<std::string>> parseCsv(std::string_view input, std::string& error)
{
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> row;
    std::string field;
    bool inQuotes = false;

    for (std::size_t index = 0; index < input.size(); ++index) {
        const char ch = input[index];
        if (inQuotes) {
            if (ch == '"') {
                if (index + 1U < input.size() && input[index + 1U] == '"') {
                    field += '"';
                    ++index;
                } else {
                    inQuotes = false;
                }
            } else {
                field += ch;
            }
            continue;
        }

        if (ch == '"') {
            if (!field.empty()) {
                error = "Unexpected quote in CSV field";
                return {};
            }
            inQuotes = true;
        } else if (ch == ',') {
            row.push_back(field);
            field.clear();
        } else if (ch == '\n') {
            row.push_back(field);
            field.clear();
            rows.push_back(row);
            row.clear();
        } else if (ch != '\r') {
            field += ch;
        }
    }

    if (inQuotes) {
        error = "Unclosed quoted CSV field";
        return {};
    }

    row.push_back(field);
    if (!(row.size() == 1U && row.front().empty() && rows.empty())) {
        rows.push_back(row);
    }
    return rows;
}

std::string csvEscape(std::string_view input)
{
    const bool quote = input.find_first_of("\",\n\r") != std::string_view::npos;
    if (!quote) {
        return std::string(input);
    }

    std::string output = "\"";
    for (char ch : input) {
        if (ch == '"') {
            output += "\"\"";
        } else {
            output += ch;
        }
    }
    output += '"';
    return output;
}

class FlatJsonParser {
public:
    explicit FlatJsonParser(std::string_view input)
        : input_(input)
    {
    }

    bool parse(std::vector<std::map<std::string, std::string>>& rows, std::string& error)
    {
        skipWhitespace();
        if (!consume('[')) {
            return fail("Expected JSON array", error);
        }
        skipWhitespace();
        if (consume(']')) {
            return true;
        }

        while (true) {
            std::map<std::string, std::string> object;
            if (!parseObject(object, error)) {
                return false;
            }
            rows.push_back(std::move(object));
            skipWhitespace();
            if (consume(']')) {
                break;
            }
            if (!consume(',')) {
                return fail("Expected ',' or ']'", error);
            }
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

    bool consume(char expected)
    {
        skipWhitespace();
        if (position_ >= input_.size() || input_[position_] != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    bool fail(std::string_view message, std::string& error)
    {
        error = std::string(message) + " at byte " + std::to_string(position_);
        return false;
    }

    bool parseObject(std::map<std::string, std::string>& object, std::string& error)
    {
        if (!consume('{')) {
            return fail("Expected object", error);
        }
        if (consume('}')) {
            return true;
        }

        while (true) {
            std::string key;
            std::string value;
            if (!parseString(key, error)) {
                return false;
            }
            if (!consume(':')) {
                return fail("Expected ':'", error);
            }
            if (!parseFlatValue(value, error)) {
                return false;
            }
            object[key] = value;

            if (consume('}')) {
                return true;
            }
            if (!consume(',')) {
                return fail("Expected ',' or '}'", error);
            }
        }
    }

    bool parseFlatValue(std::string& value, std::string& error)
    {
        skipWhitespace();
        if (position_ >= input_.size()) {
            return fail("Expected value", error);
        }
        if (input_[position_] == '"') {
            return parseString(value, error);
        }
        if (input_[position_] == '{' || input_[position_] == '[') {
            return fail("Nested values are not supported for CSV export", error);
        }

        const std::size_t start = position_;
        while (position_ < input_.size() && input_[position_] != ',' && input_[position_] != '}'
               && std::isspace(static_cast<unsigned char>(input_[position_])) == 0) {
            ++position_;
        }
        value = std::string(input_.substr(start, position_ - start));
        return !value.empty() || fail("Expected value", error);
    }

    bool parseString(std::string& value, std::string& error)
    {
        skipWhitespace();
        if (position_ >= input_.size() || input_[position_] != '"') {
            return fail("Expected string", error);
        }
        ++position_;
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
                return fail("Unsupported string escape", error);
            }
        }
        return fail("Unterminated string", error);
    }

    std::string_view input_;
    std::size_t position_ = 0;
};

} // namespace

namespace tools::csv_json {

void CsvJsonTool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline("##CsvJsonInput", input_.data(), input_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 12.0F));

    ImGui::Checkbox("First CSV row is headers", &firstRowHeaders_);
    ImGui::SameLine();
    ImGui::Checkbox("Pretty JSON", &prettyJson_);

    if (ImGui::Button("CSV to JSON")) {
        csvToJson();
    }
    ImGui::SameLine();
    if (ImGui::Button("JSON to CSV")) {
        jsonToCsv();
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
    ImGui::InputTextMultiline("##CsvJsonOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 16.0F), ImGuiInputTextFlags_ReadOnly);
}

void CsvJsonTool::csvToJson()
{
    std::string error;
    std::vector<std::vector<std::string>> rows = parseCsv(input_.data(), error);
    if (!error.empty()) {
        output_.clear();
        status_ = error;
        return;
    }
    if (rows.empty()) {
        output_.clear();
        status_ = "No CSV rows";
        return;
    }

    std::vector<std::string> headers;
    std::size_t startRow = 0;
    if (firstRowHeaders_) {
        headers = rows.front();
        startRow = 1;
    } else {
        std::size_t columns = 0;
        for (const auto& row : rows) {
            columns = std::max(columns, row.size());
        }
        for (std::size_t index = 0; index < columns; ++index) {
            headers.push_back("column_" + std::to_string(index + 1U));
        }
    }

    std::ostringstream stream;
    const char* newline = prettyJson_ ? "\n" : "";
    const char* rowIndent = prettyJson_ ? "  " : "";
    const char* fieldIndent = prettyJson_ ? "    " : "";
    const char* fieldSpace = prettyJson_ ? " " : "";

    stream << '[' << newline;
    for (std::size_t rowIndex = startRow; rowIndex < rows.size(); ++rowIndex) {
        if (rowIndex > startRow) {
            stream << ',' << newline;
        }
        stream << rowIndent << '{' << newline;
        for (std::size_t column = 0; column < headers.size(); ++column) {
            if (column > 0) {
                stream << ',' << newline;
            }
            const std::string value = column < rows[rowIndex].size() ? rows[rowIndex][column] : "";
            stream << fieldIndent << '"' << jsonEscape(headers[column]) << "\":" << fieldSpace << '"' << jsonEscape(value) << '"';
        }
        stream << newline << rowIndent << '}';
    }
    stream << newline << ']';
    output_ = stream.str();
    status_ = "Converted " + std::to_string(rows.size() - startRow) + " rows";
}

void CsvJsonTool::jsonToCsv()
{
    std::vector<std::map<std::string, std::string>> rows;
    std::string error;
    FlatJsonParser parser(input_.data());
    if (!parser.parse(rows, error)) {
        output_.clear();
        status_ = error;
        return;
    }

    std::vector<std::string> headers;
    for (const auto& row : rows) {
        for (const auto& [key, value] : row) {
            if (std::ranges::find(headers, key) == headers.end()) {
                headers.push_back(key);
            }
        }
    }

    std::ostringstream stream;
    for (std::size_t column = 0; column < headers.size(); ++column) {
        if (column > 0) {
            stream << ',';
        }
        stream << csvEscape(headers[column]);
    }
    for (const auto& row : rows) {
        stream << '\n';
        for (std::size_t column = 0; column < headers.size(); ++column) {
            if (column > 0) {
                stream << ',';
            }
            if (const auto it = row.find(headers[column]); it != row.end()) {
                stream << csvEscape(it->second);
            }
        }
    }

    output_ = stream.str();
    status_ = "Converted " + std::to_string(rows.size()) + " rows";
}

} // namespace tools::csv_json

#include "tools/sql_formatter/SqlFormatterTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace {

enum class TokenKind {
    Word,
    String,
    Comment,
    Symbol,
    Operator,
};

struct Token {
    TokenKind kind;
    std::string text;
};

std::string upperAscii(std::string_view value)
{
    std::string output;
    output.reserve(value.size());
    for (const unsigned char ch : value) {
        output += static_cast<char>(std::toupper(ch));
    }
    return output;
}

bool equalsKeyword(std::string_view value, std::string_view keyword)
{
    return upperAscii(value) == keyword;
}

bool isWordChar(char ch)
{
    return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_' || ch == '$';
}

bool isSymbol(char ch)
{
    return ch == '(' || ch == ')' || ch == ',' || ch == ';' || ch == '.';
}

std::string readQuoted(std::string_view input, std::size_t& index, char quote)
{
    const std::size_t start = index++;
    while (index < input.size()) {
        const char current = input[index++];
        if (current == quote) {
            if (index < input.size() && input[index] == quote) {
                ++index;
                continue;
            }
            return std::string(input.substr(start, index - start));
        }
        if (current == '\\' && index < input.size()) {
            ++index;
        }
    }
    return std::string(input.substr(start));
}

std::string readBracketIdentifier(std::string_view input, std::size_t& index)
{
    const std::size_t start = index++;
    while (index < input.size()) {
        const char current = input[index++];
        if (current == ']') {
            return std::string(input.substr(start, index - start));
        }
    }
    return std::string(input.substr(start));
}

std::vector<Token> tokenizeSql(std::string_view input)
{
    std::vector<Token> tokens;
    std::size_t index = 0;
    while (index < input.size()) {
        const char current = input[index];
        if (std::isspace(static_cast<unsigned char>(current)) != 0) {
            ++index;
            continue;
        }

        if ((current == '-' && index + 1U < input.size() && input[index + 1U] == '-')
            || (current == '#' && index + 1U < input.size())) {
            const std::size_t start = index;
            while (index < input.size() && input[index] != '\n' && input[index] != '\r') {
                ++index;
            }
            tokens.push_back({ TokenKind::Comment, std::string(input.substr(start, index - start)) });
            continue;
        }

        if (current == '/' && index + 1U < input.size() && input[index + 1U] == '*') {
            const std::size_t start = index;
            index += 2;
            while (index + 1U < input.size() && !(input[index] == '*' && input[index + 1U] == '/')) {
                ++index;
            }
            index = std::min(index + 2U, input.size());
            tokens.push_back({ TokenKind::Comment, std::string(input.substr(start, index - start)) });
            continue;
        }

        if (current == '\'' || current == '"' || current == '`') {
            tokens.push_back({ TokenKind::String, readQuoted(input, index, current) });
            continue;
        }

        if (current == '[') {
            tokens.push_back({ TokenKind::String, readBracketIdentifier(input, index) });
            continue;
        }

        if (isWordChar(current)) {
            const std::size_t start = index;
            while (index < input.size() && isWordChar(input[index])) {
                ++index;
            }
            tokens.push_back({ TokenKind::Word, std::string(input.substr(start, index - start)) });
            continue;
        }

        if (isSymbol(current)) {
            tokens.push_back({ TokenKind::Symbol, std::string(1, current) });
            ++index;
            continue;
        }

        const std::size_t start = index;
        while (index < input.size()
            && std::isspace(static_cast<unsigned char>(input[index])) == 0
            && !isWordChar(input[index])
            && !isSymbol(input[index])
            && input[index] != '\''
            && input[index] != '"'
            && input[index] != '`'
            && input[index] != '[') {
            ++index;
        }
        tokens.push_back({ TokenKind::Operator, std::string(input.substr(start, index - start)) });
    }
    return tokens;
}

bool isClauseKeyword(const std::vector<Token>& tokens, std::size_t index)
{
    if (tokens[index].kind != TokenKind::Word) {
        return false;
    }

    const std::string word = upperAscii(tokens[index].text);
    if (word == "SELECT" || word == "FROM" || word == "WHERE" || word == "HAVING"
        || word == "LIMIT" || word == "OFFSET" || word == "RETURNING" || word == "VALUES"
        || word == "SET" || word == "ON" || word == "UNION" || word == "EXCEPT" || word == "INTERSECT") {
        return true;
    }

    if (index + 1U >= tokens.size() || tokens[index + 1U].kind != TokenKind::Word) {
        return false;
    }

    const std::string next = upperAscii(tokens[index + 1U].text);
    return (word == "GROUP" && next == "BY")
        || (word == "ORDER" && next == "BY")
        || (word == "PARTITION" && next == "BY")
        || (word == "LEFT" && next == "JOIN")
        || (word == "RIGHT" && next == "JOIN")
        || (word == "INNER" && next == "JOIN")
        || (word == "OUTER" && next == "JOIN")
        || (word == "CROSS" && next == "JOIN")
        || (word == "FULL" && next == "JOIN");
}

bool isJoinKeyword(const Token& token)
{
    return token.kind == TokenKind::Word && equalsKeyword(token.text, "JOIN");
}

void rtrimSpaces(std::string& output)
{
    while (!output.empty() && output.back() == ' ') {
        output.pop_back();
    }
}

void appendIndent(std::string& output, int depth, int indentWidth)
{
    output.append(static_cast<std::size_t>(std::max(depth, 0) * indentWidth), ' ');
}

void newline(std::string& output, int depth, int indentWidth)
{
    rtrimSpaces(output);
    if (!output.empty() && output.back() != '\n') {
        output += '\n';
    }
    appendIndent(output, depth, indentWidth);
}

void appendSpaceIfNeeded(std::string& output)
{
    if (!output.empty() && output.back() != ' ' && output.back() != '\n' && output.back() != '(' && output.back() != '.') {
        output += ' ';
    }
}

std::string displayToken(const Token& token)
{
    if (token.kind == TokenKind::Word) {
        const std::string upper = upperAscii(token.text);
        if (upper == "SELECT" || upper == "FROM" || upper == "WHERE" || upper == "GROUP"
            || upper == "BY" || upper == "ORDER" || upper == "HAVING" || upper == "LIMIT"
            || upper == "OFFSET" || upper == "JOIN" || upper == "LEFT" || upper == "RIGHT"
            || upper == "INNER" || upper == "OUTER" || upper == "CROSS" || upper == "FULL"
            || upper == "ON" || upper == "AND" || upper == "OR" || upper == "INSERT"
            || upper == "INTO" || upper == "UPDATE" || upper == "DELETE" || upper == "VALUES"
            || upper == "SET" || upper == "CREATE" || upper == "ALTER" || upper == "DROP"
            || upper == "TABLE" || upper == "VIEW" || upper == "AS" || upper == "UNION"
            || upper == "EXCEPT" || upper == "INTERSECT" || upper == "RETURNING") {
            return upper;
        }
    }
    return token.text;
}

std::string beautifySql(std::string_view input, int indentWidth)
{
    const std::vector<Token> tokens = tokenizeSql(input);
    std::string output;
    int depth = 0;
    bool lineStart = true;

    for (std::size_t index = 0; index < tokens.size(); ++index) {
        const Token& token = tokens[index];
        const std::string text = displayToken(token);

        if (token.kind == TokenKind::Comment) {
            newline(output, depth, indentWidth);
            output += text;
            newline(output, depth, indentWidth);
            lineStart = true;
            continue;
        }

        if (text == ")") {
            depth = std::max(0, depth - 1);
            rtrimSpaces(output);
            if (!output.empty() && output.back() == '\n') {
                appendIndent(output, depth, indentWidth);
            }
            output += ')';
            lineStart = false;
            continue;
        }

        if (isClauseKeyword(tokens, index) || isJoinKeyword(token)) {
            newline(output, depth, indentWidth);
            lineStart = false;
        } else if ((equalsKeyword(text, "AND") || equalsKeyword(text, "OR")) && depth > 0) {
            newline(output, depth, indentWidth);
            lineStart = false;
        }

        if (text == ",") {
            rtrimSpaces(output);
            output += ',';
            newline(output, depth, indentWidth);
            lineStart = true;
            continue;
        }

        if (text == ";") {
            rtrimSpaces(output);
            output += ';';
            if (index + 1U < tokens.size()) {
                output += "\n\n";
                appendIndent(output, depth, indentWidth);
            }
            lineStart = true;
            continue;
        }

        if (text == ".") {
            rtrimSpaces(output);
            output += '.';
            lineStart = false;
            continue;
        }

        if (text == "(") {
            rtrimSpaces(output);
            output += '(';
            ++depth;
            lineStart = false;
            continue;
        }

        if (!lineStart) {
            appendSpaceIfNeeded(output);
        }
        output += text;
        lineStart = false;
    }

    rtrimSpaces(output);
    return output;
}

std::string minifySql(std::string_view input)
{
    const std::vector<Token> tokens = tokenizeSql(input);
    std::string output;
    for (const Token& token : tokens) {
        const std::string text = token.text;
        if (text == "," || text == ";" || text == ")") {
            rtrimSpaces(output);
            output += text;
            if (text == ";") {
                output += ' ';
            }
            continue;
        }
        if (text == "." || text == "(") {
            rtrimSpaces(output);
            output += text;
            continue;
        }
        appendSpaceIfNeeded(output);
        output += text;
    }
    rtrimSpaces(output);
    return output;
}

} // namespace

namespace tools::sql_formatter {

void SqlFormatterTool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline(
        "##SqlInput",
        input_.data(),
        input_.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 14.0F)
    );

    ImGui::SetNextItemWidth(120.0F);
    ImGui::InputInt("Indent spaces", &indentWidth_);
    indentWidth_ = std::clamp(indentWidth_, 1, 8);

    if (ImGui::Button("Beautify")) {
        output_ = beautifySql(input_.data(), indentWidth_);
        status_ = "Beautified";
    }
    ImGui::SameLine();
    if (ImGui::Button("Minify")) {
        output_ = minifySql(input_.data());
        status_ = "Minified";
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
        "##SqlOutput",
        outputBuffer.data(),
        outputBuffer.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 16.0F),
        ImGuiInputTextFlags_ReadOnly
    );
}

} // namespace tools::sql_formatter

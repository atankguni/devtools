#include "tools/url_parser/UrlParserTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <sstream>
#include <string_view>
#include <vector>

namespace {

int hexValue(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

std::string decodeComponent(std::string_view input)
{
    std::string output;
    for (std::size_t index = 0; index < input.size(); ++index) {
        if (input[index] == '+') {
            output += ' ';
            continue;
        }
        if (input[index] == '%' && index + 2U < input.size()) {
            const int high = hexValue(input[index + 1U]);
            const int low = hexValue(input[index + 2U]);
            if (high >= 0 && low >= 0) {
                output += static_cast<char>((high << 4) | low);
                index += 2U;
                continue;
            }
        }
        output += input[index];
    }
    return output;
}

std::string field(std::string_view label, std::string_view value)
{
    return std::string(label) + ": " + std::string(value) + '\n';
}

} // namespace

namespace tools::url_parser {

void UrlParserTool::draw()
{
    ImGui::TextUnformatted("URL");
    ImGui::InputTextMultiline("##UrlParserInput", input_.data(), input_.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 6.0F));

    if (ImGui::Button("Parse")) {
        parse();
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
    ImGui::TextUnformatted("Parts");
    if (!output_.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Copy Output")) {
            ui::copyToClipboard(output_.c_str());
            status_ = "Copied output";
        }
    }

    std::vector<char> buffer(output_.begin(), output_.end());
    buffer.push_back('\0');
    ImGui::InputTextMultiline("##UrlParserOutput", buffer.data(), buffer.size(), ImVec2(-1.0F, ImGui::GetTextLineHeight() * 18.0F), ImGuiInputTextFlags_ReadOnly);
}

void UrlParserTool::parse()
{
    const std::string_view url(input_.data());
    output_.clear();
    if (url.empty()) {
        status_ = "No URL";
        return;
    }

    std::string_view rest = url;
    std::string_view scheme;
    const std::size_t schemeEnd = rest.find("://");
    if (schemeEnd != std::string_view::npos) {
        scheme = rest.substr(0, schemeEnd);
        rest.remove_prefix(schemeEnd + 3U);
    }

    std::string_view fragment;
    if (const std::size_t hash = rest.find('#'); hash != std::string_view::npos) {
        fragment = rest.substr(hash + 1U);
        rest = rest.substr(0, hash);
    }

    std::string_view query;
    if (const std::size_t question = rest.find('?'); question != std::string_view::npos) {
        query = rest.substr(question + 1U);
        rest = rest.substr(0, question);
    }

    std::string_view authority;
    std::string_view path;
    if (scheme.empty()) {
        path = rest;
    } else if (const std::size_t slash = rest.find('/'); slash != std::string_view::npos) {
        authority = rest.substr(0, slash);
        path = rest.substr(slash);
    } else {
        authority = rest;
        path = "/";
    }

    std::string_view userinfo;
    std::string_view hostPort = authority;
    if (const std::size_t at = authority.rfind('@'); at != std::string_view::npos) {
        userinfo = authority.substr(0, at);
        hostPort = authority.substr(at + 1U);
    }

    std::string_view host = hostPort;
    std::string_view port;
    if (!hostPort.empty() && hostPort.front() == '[') {
        if (const std::size_t close = hostPort.find(']'); close != std::string_view::npos) {
            host = hostPort.substr(0, close + 1U);
            if (close + 2U <= hostPort.size() && hostPort[close + 1U] == ':') {
                port = hostPort.substr(close + 2U);
            }
        }
    } else if (const std::size_t colon = hostPort.rfind(':'); colon != std::string_view::npos) {
        host = hostPort.substr(0, colon);
        port = hostPort.substr(colon + 1U);
    }

    std::ostringstream stream;
    stream << field("Scheme", scheme)
           << field("Authority", authority)
           << field("User info", decodeComponent(userinfo))
           << field("Host", host)
           << field("Port", port)
           << field("Path", decodeComponent(path))
           << field("Query", query)
           << field("Fragment", decodeComponent(fragment));

    if (!query.empty()) {
        stream << "\nQuery parameters:\n";
        std::size_t start = 0;
        while (start <= query.size()) {
            const std::size_t amp = query.find('&', start);
            const std::size_t end = amp == std::string_view::npos ? query.size() : amp;
            const std::string_view pair = query.substr(start, end - start);
            const std::size_t equals = pair.find('=');
            const std::string key = decodeComponent(pair.substr(0, equals));
            const std::string value = equals == std::string_view::npos ? "" : decodeComponent(pair.substr(equals + 1U));
            stream << "- " << key << " = " << value << '\n';
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1U;
        }
    }

    output_ = stream.str();
    status_ = "Parsed";
}

} // namespace tools::url_parser

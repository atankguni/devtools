#pragma once

#include <array>
#include <string>

namespace tools::url_parser {

class UrlParserTool {
public:
    void draw();

private:
    void parse();

    std::array<char, 65536> input_ {};
    std::string output_;
    std::string status_;
};

} // namespace tools::url_parser

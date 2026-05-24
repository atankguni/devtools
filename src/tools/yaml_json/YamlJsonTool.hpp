#pragma once

#include <array>
#include <string>

namespace tools::yaml_json {

class YamlJsonTool {
public:
    void draw();

private:
    void yamlToJson();
    void jsonToYaml();

    std::array<char, 131072> input_ {};
    std::string output_;
    std::string status_;
};

} // namespace tools::yaml_json

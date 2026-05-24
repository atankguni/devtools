#pragma once

#include <array>
#include <string>

namespace tools::csv_json {

class CsvJsonTool {
public:
    void draw();

private:
    void csvToJson();
    void jsonToCsv();

    std::array<char, 131072> input_ {};
    bool firstRowHeaders_ = true;
    bool prettyJson_ = true;
    std::string output_;
    std::string status_;
};

} // namespace tools::csv_json

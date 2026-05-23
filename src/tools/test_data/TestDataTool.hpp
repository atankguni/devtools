#pragma once

#include <string>

namespace tools::test_data {

class TestDataTool {
public:
    void draw();

private:
    void generate();

    int count_ = 5;
    bool includeJson_ = true;
    bool includeEmails_ = true;
    bool includeNames_ = true;
    std::string output_;
    std::string status_;
};

} // namespace tools::test_data

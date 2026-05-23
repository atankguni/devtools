#pragma once

#include <array>
#include <string>

namespace tools::cron {

class CronTool {
public:
    CronTool();

    void draw();

private:
    std::array<char, 128> expression_ {};
    std::string output_;
    std::string status_;
};

} // namespace tools::cron

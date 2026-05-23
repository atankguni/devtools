#pragma once

#include <array>
#include <string>

namespace tools::jwt {

class JwtTool {
public:
    JwtTool();

    void draw();

private:
    enum class Mode {
        Decode,
        Encode,
    };

    Mode mode_ = Mode::Decode;
    std::array<char, 8192> tokenInput_ {};
    std::array<char, 4096> headerInput_ {};
    std::array<char, 8192> payloadInput_ {};
    std::string decodedHeader_;
    std::string decodedPayload_;
    std::string encodedToken_;
    std::string status_;
};

} // namespace tools::jwt

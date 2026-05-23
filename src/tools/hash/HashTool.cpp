#include "tools/hash/HashTool.hpp"

#include "ui/Clipboard.hpp"

#include <imgui.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

template <typename Value>
Value rotateLeft(Value value, unsigned int bits)
{
    return static_cast<Value>((value << bits) | (value >> ((sizeof(Value) * 8U) - bits)));
}

template <typename Value>
Value rotateRight(Value value, unsigned int bits)
{
    return static_cast<Value>((value >> bits) | (value << ((sizeof(Value) * 8U) - bits)));
}

std::string toHex(const std::vector<unsigned char>& bytes)
{
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (const unsigned char byte : bytes) {
        stream << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return stream.str();
}

void appendBigEndian64(std::vector<unsigned char>& data, std::uint64_t value)
{
    for (int shift = 56; shift >= 0; shift -= 8) {
        data.push_back(static_cast<unsigned char>((value >> static_cast<unsigned int>(shift)) & 0xFFU));
    }
}

std::string md5(std::string_view input)
{
    static constexpr std::array<std::uint32_t, 64> shifts {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
    };

    static constexpr std::array<std::uint32_t, 64> constants {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
    };

    std::vector<unsigned char> data(input.begin(), input.end());
    const std::uint64_t bitLength = static_cast<std::uint64_t>(data.size()) * 8U;
    data.push_back(0x80U);
    while (data.size() % 64U != 56U) {
        data.push_back(0U);
    }
    for (unsigned int shift = 0; shift < 64U; shift += 8U) {
        data.push_back(static_cast<unsigned char>((bitLength >> shift) & 0xFFU));
    }

    std::uint32_t a0 = 0x67452301;
    std::uint32_t b0 = 0xefcdab89;
    std::uint32_t c0 = 0x98badcfe;
    std::uint32_t d0 = 0x10325476;

    for (std::size_t offset = 0; offset < data.size(); offset += 64U) {
        std::array<std::uint32_t, 16> words {};
        for (std::size_t index = 0; index < words.size(); ++index) {
            const std::size_t pos = offset + index * 4U;
            words[index] = static_cast<std::uint32_t>(data[pos])
                | (static_cast<std::uint32_t>(data[pos + 1U]) << 8U)
                | (static_cast<std::uint32_t>(data[pos + 2U]) << 16U)
                | (static_cast<std::uint32_t>(data[pos + 3U]) << 24U);
        }

        std::uint32_t a = a0;
        std::uint32_t b = b0;
        std::uint32_t c = c0;
        std::uint32_t d = d0;

        for (std::uint32_t index = 0; index < 64U; ++index) {
            std::uint32_t f = 0;
            std::uint32_t g = 0;
            if (index < 16U) {
                f = (b & c) | (~b & d);
                g = index;
            } else if (index < 32U) {
                f = (d & b) | (~d & c);
                g = (5U * index + 1U) % 16U;
            } else if (index < 48U) {
                f = b ^ c ^ d;
                g = (3U * index + 5U) % 16U;
            } else {
                f = c ^ (b | ~d);
                g = (7U * index) % 16U;
            }

            const std::uint32_t next = d;
            d = c;
            c = b;
            b += rotateLeft<std::uint32_t>(a + f + constants[index] + words[g], shifts[index]);
            a = next;
        }

        a0 += a;
        b0 += b;
        c0 += c;
        d0 += d;
    }

    std::vector<unsigned char> digest;
    for (const std::uint32_t value : { a0, b0, c0, d0 }) {
        for (unsigned int shift = 0; shift < 32U; shift += 8U) {
            digest.push_back(static_cast<unsigned char>((value >> shift) & 0xFFU));
        }
    }
    return toHex(digest);
}

std::string sha1(std::string_view input)
{
    std::vector<unsigned char> data(input.begin(), input.end());
    const std::uint64_t bitLength = static_cast<std::uint64_t>(data.size()) * 8U;
    data.push_back(0x80U);
    while (data.size() % 64U != 56U) {
        data.push_back(0U);
    }
    appendBigEndian64(data, bitLength);

    std::uint32_t h0 = 0x67452301;
    std::uint32_t h1 = 0xefcdab89;
    std::uint32_t h2 = 0x98badcfe;
    std::uint32_t h3 = 0x10325476;
    std::uint32_t h4 = 0xc3d2e1f0;

    for (std::size_t offset = 0; offset < data.size(); offset += 64U) {
        std::array<std::uint32_t, 80> words {};
        for (std::size_t index = 0; index < 16U; ++index) {
            const std::size_t pos = offset + index * 4U;
            words[index] = (static_cast<std::uint32_t>(data[pos]) << 24U)
                | (static_cast<std::uint32_t>(data[pos + 1U]) << 16U)
                | (static_cast<std::uint32_t>(data[pos + 2U]) << 8U)
                | static_cast<std::uint32_t>(data[pos + 3U]);
        }
        for (std::size_t index = 16U; index < words.size(); ++index) {
            words[index] = rotateLeft<std::uint32_t>(
                words[index - 3U] ^ words[index - 8U] ^ words[index - 14U] ^ words[index - 16U],
                1U
            );
        }

        std::uint32_t a = h0;
        std::uint32_t b = h1;
        std::uint32_t c = h2;
        std::uint32_t d = h3;
        std::uint32_t e = h4;

        for (std::size_t index = 0; index < words.size(); ++index) {
            std::uint32_t f = 0;
            std::uint32_t k = 0;
            if (index < 20U) {
                f = (b & c) | (~b & d);
                k = 0x5a827999;
            } else if (index < 40U) {
                f = b ^ c ^ d;
                k = 0x6ed9eba1;
            } else if (index < 60U) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8f1bbcdc;
            } else {
                f = b ^ c ^ d;
                k = 0xca62c1d6;
            }

            const std::uint32_t temp = rotateLeft<std::uint32_t>(a, 5U) + f + e + k + words[index];
            e = d;
            d = c;
            c = rotateLeft<std::uint32_t>(b, 30U);
            b = a;
            a = temp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    std::vector<unsigned char> digest;
    for (const std::uint32_t value : { h0, h1, h2, h3, h4 }) {
        for (int shift = 24; shift >= 0; shift -= 8) {
            digest.push_back(static_cast<unsigned char>((value >> static_cast<unsigned int>(shift)) & 0xFFU));
        }
    }
    return toHex(digest);
}

std::vector<unsigned char> sha256Bytes(std::string_view input)
{
    static constexpr std::array<std::uint32_t, 64> constants {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
    };

    std::vector<unsigned char> data(input.begin(), input.end());
    const std::uint64_t bitLength = static_cast<std::uint64_t>(data.size()) * 8U;
    data.push_back(0x80U);
    while (data.size() % 64U != 56U) {
        data.push_back(0U);
    }
    appendBigEndian64(data, bitLength);

    std::array<std::uint32_t, 8> hash {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
    };

    for (std::size_t offset = 0; offset < data.size(); offset += 64U) {
        std::array<std::uint32_t, 64> words {};
        for (std::size_t index = 0; index < 16U; ++index) {
            const std::size_t pos = offset + index * 4U;
            words[index] = (static_cast<std::uint32_t>(data[pos]) << 24U)
                | (static_cast<std::uint32_t>(data[pos + 1U]) << 16U)
                | (static_cast<std::uint32_t>(data[pos + 2U]) << 8U)
                | static_cast<std::uint32_t>(data[pos + 3U]);
        }
        for (std::size_t index = 16U; index < words.size(); ++index) {
            const std::uint32_t s0 = rotateRight<std::uint32_t>(words[index - 15U], 7U)
                ^ rotateRight<std::uint32_t>(words[index - 15U], 18U)
                ^ (words[index - 15U] >> 3U);
            const std::uint32_t s1 = rotateRight<std::uint32_t>(words[index - 2U], 17U)
                ^ rotateRight<std::uint32_t>(words[index - 2U], 19U)
                ^ (words[index - 2U] >> 10U);
            words[index] = words[index - 16U] + s0 + words[index - 7U] + s1;
        }

        std::uint32_t a = hash[0];
        std::uint32_t b = hash[1];
        std::uint32_t c = hash[2];
        std::uint32_t d = hash[3];
        std::uint32_t e = hash[4];
        std::uint32_t f = hash[5];
        std::uint32_t g = hash[6];
        std::uint32_t h = hash[7];

        for (std::size_t index = 0; index < words.size(); ++index) {
            const std::uint32_t s1 = rotateRight<std::uint32_t>(e, 6U)
                ^ rotateRight<std::uint32_t>(e, 11U)
                ^ rotateRight<std::uint32_t>(e, 25U);
            const std::uint32_t choice = (e & f) ^ (~e & g);
            const std::uint32_t temp1 = h + s1 + choice + constants[index] + words[index];
            const std::uint32_t s0 = rotateRight<std::uint32_t>(a, 2U)
                ^ rotateRight<std::uint32_t>(a, 13U)
                ^ rotateRight<std::uint32_t>(a, 22U);
            const std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t temp2 = s0 + majority;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        hash[0] += a;
        hash[1] += b;
        hash[2] += c;
        hash[3] += d;
        hash[4] += e;
        hash[5] += f;
        hash[6] += g;
        hash[7] += h;
    }

    std::vector<unsigned char> digest;
    for (const std::uint32_t value : hash) {
        for (int shift = 24; shift >= 0; shift -= 8) {
            digest.push_back(static_cast<unsigned char>((value >> static_cast<unsigned int>(shift)) & 0xFFU));
        }
    }
    return digest;
}

std::vector<unsigned char> sha512Bytes(std::string_view input, bool sha384)
{
    static constexpr std::array<std::uint64_t, 80> constants {
        0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
        0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
        0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
        0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
        0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
        0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
        0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
        0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
        0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
        0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
        0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
        0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
        0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
        0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
        0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
        0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
        0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
        0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
        0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
        0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL,
    };

    std::vector<unsigned char> data(input.begin(), input.end());
    const std::uint64_t bitLengthLow = static_cast<std::uint64_t>(data.size()) * 8U;
    data.push_back(0x80U);
    while (data.size() % 128U != 112U) {
        data.push_back(0U);
    }
    appendBigEndian64(data, 0U);
    appendBigEndian64(data, bitLengthLow);

    std::array<std::uint64_t, 8> hash = sha384
        ? std::array<std::uint64_t, 8> {
            0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL, 0x9159015a3070dd17ULL, 0x152fecd8f70e5939ULL,
            0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL, 0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL,
        }
        : std::array<std::uint64_t, 8> {
            0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
            0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL,
        };

    for (std::size_t offset = 0; offset < data.size(); offset += 128U) {
        std::array<std::uint64_t, 80> words {};
        for (std::size_t index = 0; index < 16U; ++index) {
            const std::size_t pos = offset + index * 8U;
            for (std::size_t byte = 0; byte < 8U; ++byte) {
                words[index] = (words[index] << 8U) | data[pos + byte];
            }
        }
        for (std::size_t index = 16U; index < words.size(); ++index) {
            const std::uint64_t s0 = rotateRight<std::uint64_t>(words[index - 15U], 1U)
                ^ rotateRight<std::uint64_t>(words[index - 15U], 8U)
                ^ (words[index - 15U] >> 7U);
            const std::uint64_t s1 = rotateRight<std::uint64_t>(words[index - 2U], 19U)
                ^ rotateRight<std::uint64_t>(words[index - 2U], 61U)
                ^ (words[index - 2U] >> 6U);
            words[index] = words[index - 16U] + s0 + words[index - 7U] + s1;
        }

        std::array<std::uint64_t, 8> work = hash;
        for (std::size_t index = 0; index < words.size(); ++index) {
            const std::uint64_t s1 = rotateRight<std::uint64_t>(work[4], 14U)
                ^ rotateRight<std::uint64_t>(work[4], 18U)
                ^ rotateRight<std::uint64_t>(work[4], 41U);
            const std::uint64_t choice = (work[4] & work[5]) ^ (~work[4] & work[6]);
            const std::uint64_t temp1 = work[7] + s1 + choice + constants[index] + words[index];
            const std::uint64_t s0 = rotateRight<std::uint64_t>(work[0], 28U)
                ^ rotateRight<std::uint64_t>(work[0], 34U)
                ^ rotateRight<std::uint64_t>(work[0], 39U);
            const std::uint64_t majority = (work[0] & work[1]) ^ (work[0] & work[2]) ^ (work[1] & work[2]);
            const std::uint64_t temp2 = s0 + majority;

            work[7] = work[6];
            work[6] = work[5];
            work[5] = work[4];
            work[4] = work[3] + temp1;
            work[3] = work[2];
            work[2] = work[1];
            work[1] = work[0];
            work[0] = temp1 + temp2;
        }

        for (std::size_t index = 0; index < hash.size(); ++index) {
            hash[index] += work[index];
        }
    }

    std::vector<unsigned char> digest;
    const std::size_t wordCount = sha384 ? 6U : 8U;
    for (std::size_t index = 0; index < wordCount; ++index) {
        for (int shift = 56; shift >= 0; shift -= 8) {
            digest.push_back(static_cast<unsigned char>((hash[index] >> static_cast<unsigned int>(shift)) & 0xFFU));
        }
    }
    return digest;
}

void drawHashRow(const char* label, const std::string& value)
{
    ImGui::TextUnformatted(label);
    ImGui::SameLine(86.0F);
    ImGui::PushID(label);
    if (ImGui::Button("Copy")) {
        ui::copyToClipboard(value.c_str());
    }
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::TextUnformatted(value.c_str());
}

} // namespace

namespace tools::hash {

void HashTool::draw()
{
    ImGui::TextUnformatted("Input");
    ImGui::InputTextMultiline(
        "##HashInput",
        input_.data(),
        input_.size(),
        ImVec2(-1.0F, ImGui::GetTextLineHeight() * 10.0F)
    );

    if (ImGui::Button("Generate Hashes")) {
        generate();
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        input_.fill('\0');
        md5_.clear();
        sha1_.clear();
        sha256_.clear();
        sha384_.clear();
        sha512_.clear();
        status_.clear();
    }

    if (!status_.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", status_.c_str());
    }

    if (!sha256_.empty()) {
        ImGui::Separator();
        if (ImGui::Button("Copy All")) {
            const std::string all = "MD5: " + md5_ + "\nSHA-1: " + sha1_ + "\nSHA-256: " + sha256_
                + "\nSHA-384: " + sha384_ + "\nSHA-512: " + sha512_;
            ui::copyToClipboard(all.c_str());
            status_ = "Copied all hashes";
        }
        ImGui::TextDisabled("MD5 and SHA-1 are legacy hashes. Prefer SHA-256 or SHA-512 for new work.");
        drawHashRow("MD5", md5_);
        drawHashRow("SHA-1", sha1_);
        drawHashRow("SHA-256", sha256_);
        drawHashRow("SHA-384", sha384_);
        drawHashRow("SHA-512", sha512_);
    }
}

void HashTool::generate()
{
    const std::string_view input(input_.data());
    md5_ = md5(input);
    sha1_ = sha1(input);
    sha256_ = toHex(sha256Bytes(input));
    sha384_ = toHex(sha512Bytes(input, true));
    sha512_ = toHex(sha512Bytes(input, false));
    status_ = "Generated";
}

} // namespace tools::hash

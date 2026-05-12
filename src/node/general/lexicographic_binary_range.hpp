#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

struct LexicographicByteRange {
    std::vector<uint8_t> begin;
    std::optional<std::vector<uint8_t>> end;

    static std::optional<LexicographicByteRange>
    from_hex(std::string_view hexPrefix);

private:
    LexicographicByteRange() { };
};

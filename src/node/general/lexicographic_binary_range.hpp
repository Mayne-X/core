#pragma once
#include "wrt/optional.hpp"

#include <cstdint>
#include <vector>
struct LexicographicByteRange {
    std::vector<uint8_t> begin;
    wrt::optional<std::vector<uint8_t>> end;

    static wrt::optional<LexicographicByteRange>
    from_hex(std::string_view hexPrefix);

private:
    LexicographicByteRange() { };
};

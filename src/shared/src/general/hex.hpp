#pragma once
#include "errors.hpp"
#include "general/byte_order.hpp"
#include "wrt/hex_ref.hxx"
#include <span>
#include <string>

inline void serialize_hex(std::span<const uint8_t> in, std::output_iterator<char> auto out)
{
    constexpr const char* h = "0123456789abcdef";
    for (auto u : in) {
        *(out++) = h[u >> 4];
        *(out++) = h[u & 15];
    }
}

void serialize_hex(uint32_t number, std::output_iterator<char> auto out)
{
    uint32_t tmp = hton32(number);
    serialize_hex({ (const uint8_t*)&tmp, 4 }, out);
}

std::string serialize_hex(std::span<const uint8_t> in);
std::string serialize_hex(uint32_t v);

inline auto generate_invhex()
{
    return Error(EINV_HEX);
}

using HexRef = wrt::hex_view<uint8_t, generate_invhex>;

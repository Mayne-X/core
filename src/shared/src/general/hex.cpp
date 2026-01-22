#include "hex.hpp"
#include "hex_digit.hpp"

void serialize_hex(uint32_t number, char* out)
{
    uint32_t tmp = hton32(number);
    serialize_hex((const uint8_t*)&tmp, 4, out);
};

void serialize_hex(const uint8_t* data, size_t size, char* out)
{
    constexpr const char* h = "0123456789abcdef";
    for (size_t i = 0; i < size; ++i) {
        out[2 * i] = h[data[i] >> 4];
        out[2 * i + 1] = h[data[i] & 15];
    }
}

std::string serialize_hex(const uint8_t* data, size_t size)
{
    std::string out;
    out.resize(2 * size);
    serialize_hex(data, size, out.data());
    return out;
}


bool parse_hex(std::string_view in, uint8_t* out, size_t out_size)
{
    if (in.size() != out_size * 2)
        return false;
    bool valid = true;
    for (size_t i = 0; i < out_size && valid; ++i) {
        out[i] = (hex_digit(in[2 * i], valid) << 4)
            + (hex_digit(in[2 * i + 1], valid));
    }
    return valid;
}

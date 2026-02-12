#include <ranges>

namespace wrt {
[[nodiscard]] inline constexpr unsigned char hex_digit(char c, bool& valid)
{
    switch (c) {
    case '0':
        return 0;
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'a':
    case 'A':
        return 10;
    case 'b':
    case 'B':
        return 11;
    case 'c':
    case 'C':
        return 12;
    case 'd':
    case 'D':
        return 13;
    case 'e':
    case 'E':
        return 14;
    case 'f':
    case 'F':
        return 15;
    default:
        valid = false;
        return 0;
    }
}

template <typename ByteType, auto generate_exception>
requires(sizeof(ByteType) == 1)
class hex_view : public std::ranges::view_interface<
                     hex_view<ByteType, generate_exception>> {
private:
    std::string_view hexStr;
    static void throw_exception() { throw generate_exception(); }

public:
    constexpr hex_view(std::string_view hexStr)
        : hexStr(hexStr)
    {
        if (hexStr.size() % 2 != 0)
            throw_exception();
    }
    constexpr bool insert_into(std::output_iterator<ByteType> auto iter) const
    {
        bool valid = true;
        for (size_t i = 0; i < hexStr.size() / 2; ++i) {
            *(iter) = ByteType((hex_digit(hexStr[2 * i], valid) << 4) + (hex_digit(hexStr[2 * i + 1], valid)));
            ++iter;
        }
        return valid;
    }
    [[nodiscard]] constexpr bool parse_to(std::span<ByteType> out) const
    {
        if (hexStr.size() != out.size() * 2)
            return false;
        return insert_into(out.begin());
    }

    constexpr void place_into_throw(std::span<ByteType> out) const
    {
        if (!parse_to(out))
            throw_exception();
    }

    constexpr void
    place_into_throw(std::output_iterator<ByteType> auto iter) const
    {
        if (!insert_into(iter))
            throw_exception();
    }
    template <size_t N>
    constexpr operator std::array<ByteType, N>() const
    {
        std::array<ByteType, N> out;
        place_into_throw(std::span(out));
        return out;
    }
    class iterator {
    private:
        std::string_view::iterator pos;
        constexpr iterator(std::string_view::iterator pos)
            : pos(pos)
        {
        }
        friend hex_view;

    public:
        iterator() { };
        using value_type = ByteType;
        using difference_type = std::ptrdiff_t;
        using reference = ByteType;
        using pointer = void;
        using iterator_category = std::forward_iterator_tag;
        constexpr iterator& operator++()
        {
            pos += 2;
            return *this;
        }
        constexpr iterator operator++(int)
        {
            auto tmp { *this };
            operator++();
            return tmp;
        }
        constexpr ByteType operator*() const
        {
            bool valid { true };
            auto b { ByteType((hex_digit(*pos, valid) << 4) | (hex_digit(*(pos + 1), valid))) };
            if (!valid)
                throw_exception();
            return b;
        }
        constexpr auto operator<=>(const iterator&) const = default;
        constexpr bool operator==(const iterator&) const = default;
        constexpr bool operator!=(const iterator&) const = default;
    };
    constexpr iterator begin() const { return hexStr.begin(); };
    constexpr iterator end() const { return hexStr.end(); };
    constexpr size_t size() const { return hexStr.size() / 2; }
};

}

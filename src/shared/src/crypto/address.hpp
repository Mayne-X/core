#pragma once
#include "general/result.hpp"
#include "general/view.hpp"
#include <array>
#include <string>

class AddressView : public View<20> {
public:
    explicit AddressView(const uint8_t* pos)
        : View(pos) { };
    std::array<uint8_t, 24> serialize() const;
    std::string to_string() const;
};
namespace wrt {
template <size_t N>
struct byte_arr : public std::array<uint8_t, N> {
    using parent = std::array<uint8_t, N>;
    byte_arr(parent a)
        : parent(std::move(a))
    {
    }
    using parent::parent;
    using parent::size;
};

}

class Address : public std::array<uint8_t, 20> {
    static constexpr size_t N = 20;
    Address() { };

public:
    static constexpr size_t byte_size() { return N; }
    friend class PubKey;
    using std::array<uint8_t, N>::array;
    static Address uninitialized() { return {}; }
    constexpr Address(std::array<uint8_t, N> s)
        : std::array<uint8_t, N>(std::move(s))
    {
    }

    static const Address zero;
    Address(const std::string_view);
    static Result<Address> parse(std::string_view);
    Address(std::span<const uint8_t, N> s);
    Address(const AddressView v)
    {
        std::copy(v.begin(), v.end(), data());
    }
    operator AddressView() const
    {
        return AddressView(data());
    }
    std::string to_string() const { return static_cast<AddressView>(*this).to_string(); }
    bool operator==(const AddressView other) const { return static_cast<AddressView>(*this) == other; }
    bool operator==(const Address other) const { return static_cast<AddressView>(*this) == other; }
    Address& operator=(const AddressView);
    std::array<uint8_t, 24> serialize() const { return static_cast<AddressView>(*this).serialize(); }
    struct Comparator {
        using is_transparent = std::true_type;
        static constexpr size_t offsetHash = 99;
        inline bool operator()(const Address& i1, const Address& i2) const
        {
            return i1 < i2;
        }
        inline bool operator()(const Address& i1, const AddressView i2) const
        {
            return static_cast<AddressView>(i1) < i2;
        }
        inline bool operator()(const AddressView i1, const Address& i2) const
        {
            return i1 < static_cast<AddressView>(i2);
        }
    };
};
constexpr const Address Address::zero { std::array<uint8_t, N> { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

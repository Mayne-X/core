#pragma once
#include "general/errors.hpp"
#include "general/view.hpp"
#include "wrt/optional.hpp"
#include <array>
#include <cstring>
#include <string>

class Hash;
class HashView : public View<32> {
public:
    inline HashView(const uint8_t* pos)
        : View<32>(pos) { };
    inline bool operator==(HashView hv) const
    {
        return View::operator==(hv);
    };
    HashView(const Hash&);
};

class Hash : public std::array<uint8_t, 32> {
    Hash() = default;

public:
    static constexpr size_t byte_size() { return 32; }
    static wrt::optional<Hash> try_parse(std::string_view);
    static Hash parse_throw(std::string_view);
    static constexpr Hash zero()
    {
        return std::array<uint8_t, 32> { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    }
    static Hash uninitialized()
    {
        return {};
    }
    constexpr Hash(std::array<uint8_t, 32> other)
        : array(std::move(other))
    {
    }
    Hash(const Hash&) = default;
    Hash(Hash&&) = default;
    std::string hex_string() const;
    explicit Hash(HashView hv)
    {
        memcpy(data(), hv.data(), 32);
    }
    Hash& operator=(const Hash&) = default;
    bool operator==(const Hash&) const = default;
    bool operator!=(const Hash&) const = default;
};

inline HashView::HashView(const Hash& h)
    : HashView(h.data())
{
}

inline bool operator==(const Hash& h, const HashView& hv)
{
    return (HashView(h) == hv);
};
inline bool operator==(const HashView& hv, const Hash& h)
{
    return (HashView(h) == hv);
};

template <typename T>
class GenericHash : public Hash {
public:
    constexpr explicit GenericHash(Hash h)
        : Hash(std::move(h))
    {
    }
    explicit GenericHash(HashView h)
        : Hash(h)
    {
    }
    constexpr explicit GenericHash(std::array<uint8_t, 32> other)
        : Hash(std::move(other))
    {
    }
    [[nodiscard]] static wrt::optional<T> try_parse(std::string_view s)
    {
        auto p { Hash::try_parse(s) };
        if (p)
            return T { *p };
        return {};
    }
    [[nodiscard]] static T parse_throw(std::string_view s)
    {
        return Hash::parse_throw(s);
    }
    static T uninitialized()
    {
        return T { Hash::uninitialized() };
    }
    constexpr static T zero()
    {
        return T { Hash::zero() };
    }
};

class BlockHash : public GenericHash<BlockHash> {
public:
    using GenericHash::GenericHash;
    static BlockHash genesis();
};

class AssetHash : public GenericHash<AssetHash> {
public:
    using GenericHash::GenericHash;
    static const AssetHash WART;
    constexpr bool is_wart() const
    {
        return *this == AssetHash::WART;
    }
};
constexpr const AssetHash AssetHash::WART = { zero() };

class TxHash : public GenericHash<TxHash> {
public:
    using GenericHash::GenericHash;
};

class PinHash : public GenericHash<PinHash> {
public:
    using GenericHash::GenericHash;
};

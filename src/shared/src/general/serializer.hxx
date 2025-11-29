#pragma once

#include "general/byte_order.hpp"
#include "serializer_fwd.hxx"
#include "view_fwd.hpp"
#include "wrt/optional.hpp"
#include <cstdint>
#include <span>
#include <string_view>

struct ByteCounter {
    size_t N { 0 };
    constexpr void add_size(size_t size)
    {
        N += size;
    }
};

template <typename S, typename T>
concept Serializing = Serializer<S> && requires(S& s, const T& t) {
    { t.serialize(s) };
};

constexpr auto&& operator<<(Serializer auto&& s, std::span<const uint8_t> sp)
{
    s.write(sp);
    return std::forward<decltype(s)>(s);
}
constexpr auto& operator<<(ByteCounter& s, std::span<const uint8_t> sp)
{
    s.add_size(sp.size());
    return s;
}

constexpr auto&& operator<<(Serializer auto&& s, ByteSwappable auto v)
{
    auto valBe { network_byte_swap(v) };
    std::span<const uint8_t> sp((uint8_t*)&valBe, sizeof(v));
    return std::forward<decltype(s)>(s << sp);
}
constexpr auto& operator<<(ByteCounter& s, ByteSwappable auto v)
{
    s.add_size(sizeof(v));
    return s;
}

template <typename T>
auto&& operator<<(Serializer auto&& s, const wrt::optional<T>& o)
{
    if (o)
        return std::forward<decltype(s)>(s << uint8_t(1) << *o);
    else
        return std::forward<decltype(s)>(s << uint8_t(0));
}
template <typename T>
auto&& operator<<(ByteCounter& s, const wrt::optional<T>& o)
{
    s.add_size(1);
    if (o)
        return s << *o;
}

template <size_t N>
constexpr auto& operator<<(ByteCounter& s, const View<N>&)
{
    s.add_size(N);
    return s;
}

constexpr auto&& operator<<(Serializer auto&& s, bool b)
{
    return std::forward<decltype(s)>(s << (b ? uint8_t(1) : uint8_t(0)));
}

constexpr auto&& operator<<(Serializer auto&& s, std::string_view r)
{
    std::span sp(reinterpret_cast<const uint8_t*>(r.data()), r.size());
    return std::forward<decltype(s)>(s << sp);
}

template <Serializer S, typename T>
requires Serializing<S, T>
constexpr auto&& operator<<(S&& s, const T& t)
{
    t.serialize(s);
    return std::forward<S>(s);
}

template <typename T>
constexpr size_t count_bytes(const T& t)
{
    return (ByteCounter() << t).N;
}

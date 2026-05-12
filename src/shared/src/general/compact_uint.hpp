#pragma once
#include "general/funds.hpp"
#include <cassert>
#include <cstdint>
class Reader;
class CompactUInt;
class CompactUInt {
    static constexpr std::optional<Wart> uncompact_value(uint16_t val)
    { // OK
        uint64_t e = (val & uint64_t(0xFC00u)) >> 10; // < 2^6 = 64
        uint64_t m = (val & uint64_t(0x03FFu)) + uint64_t(0x0400u);
        if (e < 10) {
            return Wart::from_value(m >> (10 - e));
        } else {
            return Wart::from_value(m << (e - 10));
        }
    }
    constexpr CompactUInt(uint16_t val)
        : val(val)
    {
    }

public:
    CompactUInt(Reader& r);
    static consteval size_t byte_size() { return sizeof(val); }
    static constexpr auto smallest() { return CompactUInt(0); }
    static constexpr auto largest() { return CompactUInt(0xFFFFu); }
    static CompactUInt from_value_assert(uint16_t val)
    {
        auto v { from_value(val) };
        assert(v.has_value());
        return *v;
    }
    static CompactUInt from_value_throw(uint16_t val)
    {
        auto v { from_value(val) };
        if (!v)
            throw Error(EBADFEE);
        return *v;
    }
    static std::optional<CompactUInt> from_value(uint16_t val)
    {
        if (uncompact_value(val).has_value())
            return CompactUInt { val };
        return {};
    }
    constexpr Wart uncompact() const
    {
        auto v { uncompact_value(val) };
        assert(v.has_value());
        return *v;
    };
    [[nodiscard]] static Result<CompactUInt> try_parse(std::string_view s, bool ceil = false);
    auto to_string() const { return uncompact().to_string(); }
    // [[nodiscard]] static CompactUInt compact(Wart, bool ceil = false);
    [[nodiscard]] static constexpr CompactUInt compact(Wart f, bool ceil = false)
    {
        if (f.is_zero())
            return uint16_t(0x0000u);
        uint16_t e = 10;
        const uint64_t threshold = uint64_t(0x07FFu);
        uint64_t e8 { f.E8() };
        bool exact { true };
        while (e8 > threshold) {
            e += 1;
            if (ceil && ((e8 & 1) != 0)) {
                exact = false;
            }
            e8 >>= 1;
        }
        if (ceil && exact == false) {
            e8 += 1;
            if (e8 > threshold) {
                e8 >>= 1;
                e += 1;
                if (e > 53)
                    return largest();
            }
        }
        while (e8 < uint64_t(0x0400u)) {
            e -= 1;
            e8 <<= 1;
        }
        return (e << 10) | (uint16_t(e8) & uint16_t(0x03FF));
    }
    auto next() const
    {
        auto res(*this);
        res.val += 1;
        return res;
        assert(res.val != 0);
    }
    constexpr uint16_t value() const { return val; }
    void serialize(RawSerializer auto&& s) const
    {
        s << value();
    }

    // default comparison is correct even without uncompacting.
    auto operator<=>(const CompactUInt&) const = default;

private:
    uint16_t val;
};

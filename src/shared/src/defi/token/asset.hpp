#pragma once
#include "crypto/hash.hpp"
#include "general/funds.hpp"
#include "general/result.hpp"
#include "general/view.hpp"
#include "id.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <string>

class Reader;
class Writer;

struct TokenFunds {
    TokenId id;
    Funds_uint64 amount;
};

class AssetName {
    static constexpr size_t maxlen { 5 };

    explicit AssetName(std::string assetName)
        : name(std::move(assetName))
    {
    }
public:
    static bool is_valid_str(std::string_view s)
    {
        auto ascii_alnum {
            [](char c) {
                return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
            }
        };
        return s.size() > 0
            && s.size() <= maxlen
            && std::ranges::all_of(s, ascii_alnum);
    }

    static Result<AssetName> try_parse(std::string_view s){
        if (!is_valid_str(s))
            return Error(EASSETNAME);
        return AssetName(std::string(s));
    }

    std::string to_string() const
    {
        return name;
    }
    void serialize(RawSerializer auto&& s) const
    {
        for (size_t i = 0; i < maxlen; ++i)
            s << uint8_t(i < name.size() ? name[i] : 0);
    }
    static constexpr size_t byte_size() { return maxlen; }

    AssetName(View<maxlen>);
    AssetName(Reader&);
    auto& c_str() const { return name; }

private:
    std::string name;
};

struct AssetBasic {
    AssetId id;
    AssetHash hash;
    AssetName name;
    TokenPrecision precision;
};

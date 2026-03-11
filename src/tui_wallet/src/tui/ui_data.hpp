#pragma once
#include "api/types/input.hpp"
#include "crypto/hash.hpp"
#include "general/funds.hpp"
#include "general/hex.hpp"
#include "tabs_fwd.hpp"

#include <string>

struct TokenInfo {
    std::string assetName;
    api::TokenSpec spec;
    TokenDecimals assetDecimals;
    std::string market() const { return assetName + "/WART"; }
    std::string liquidity_name() const { return assetName + "-LIQUIDITY"; }
    std::string to_string() const { return spec.to_string() + " (" + assetName + ")"; }
    TokenDecimals decimals() const { return spec.isLiquidity ? TokenDecimals::LIQUIDITY : assetDecimals; };
    std::string pretty_name() const { return spec.isLiquidity ? liquidity_name() : assetName; }
    constexpr TokenInfo(std::string name, api::TokenSpec spec, TokenDecimals decimals)
        : assetName(std::move(name))
        , spec(std::move(spec))
        , assetDecimals(decimals)
    {
    }
    static const TokenInfo DEMO;
    static const TokenInfo WART;
};
inline const TokenInfo TokenInfo::DEMO { "DEMO", api::TokenSpec::WART, 8 };
inline const TokenInfo TokenInfo::WART { "WART", api::TokenSpec::WART, TokenDecimals::WART };

struct AssetInfo {
    std::string name;
    AssetHash hash;
    TokenDecimals decimals;
    TokenInfo token(bool isLiquidity)
    {
        return { name, api::TokenSpec(hash, isLiquidity), decimals };
    }
    std::string market() const { return name + "/WART"; }
    std::string liquidity_name() const { return name + "-LIQUIDITY"; }
    std::string to_string() const { return name + " (" + serialize_hex(hash) + ")"; }
    AssetInfo(std::string name, AssetHash hash, TokenDecimals decimals)
        : name(std::move(name))
        , hash(std::move(hash))
        , decimals(decimals)
    {
    }
    static AssetInfo demo()
    {
        return { "DEMO", AssetHash::zero(), 8 };
    }
};

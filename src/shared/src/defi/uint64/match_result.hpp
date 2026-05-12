#pragma once
#include "general/funds.hpp"
namespace defi {
struct BaseQuote_uint64;
struct Delta_uint64 {
    bool operator==(const Delta_uint64&) const = default;
    bool isQuote { false };
    Funds_uint64 amount;
    BaseQuote_uint64 base_quote() const;
};
struct NonzeroDelta_uint64 {
    explicit NonzeroDelta_uint64(bool isQuote, NonzeroFunds_uint64 amount)
        : isQuote_(std::move(isQuote))
        , amount_(std::move(amount))
    {
    }
    Delta_uint64 get() const { return { isQuote_, amount_ }; }
    bool is_quote() const { return isQuote_; }
    auto amount() const { return amount_; }
    bool operator==(const NonzeroDelta_uint64&) const = default;

private:
    bool isQuote_ { false };
    NonzeroFunds_uint64 amount_;
};
struct BaseQuote64 {
    Funds_uint64 base;
    Funds_uint64 quote;
    BaseQuote64(Funds_uint64 base, Funds_uint64 quote)
        : base(base)
        , quote(quote)
    {
    }
    BaseQuote64(Reader& r)
        : base(r)
        , quote(r) { };
    bool operator==(const BaseQuote64&) const = default;
};
struct FillResult_uint64 {
    std::optional<NonzeroDelta_uint64> toPool;
    BaseQuote64 filled;
    bool operator==(const FillResult_uint64&) const = default;
};

using MatchResult_uint64 = FillResult_uint64;
}

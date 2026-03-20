#pragma once
#include "block/chain/height.hpp"
#include "crypto/hash.hpp"
#include "defi/token/id.hpp"
#include "general/timestamp.hpp"
namespace market_history {
class TradeAmount {
protected:
    double _base;
    double _quote;
    TradeAmount(double base, double quote)
        : _base { base }
        , _quote { quote }
    {
    }

public:
    double base() const { return _base; }
    double quote() const { return _quote; }
    double price() const
    {
        return _quote / _base;
    }
    [[nodiscard]] static wrt::optional<TradeAmount> create(double base, double quote)
    {
        if (base == 0 || quote == 0)
            return {};
        return TradeAmount { base, quote };
    }
};
struct BlockInfo {
    NonzeroHeight height;
    BlockHash hash;
    Timestamp timestamp;
    BlockInfo(NonzeroHeight height, BlockHash hash, Timestamp timestamp)
        : height(std::move(height))
        , hash(std::move(hash))
        , timestamp(std::move(timestamp))
    {
    }
    void push_trade(AssetId assetId, TradeAmount amount)
    {
        trades.push_back({ amount, assetId });
    }
    // void insert_new_asset(AssetId id, AssetHash hash)
    // {
    //     newAssets.push_back({ id, hash });
    // }

    // protected:
    struct NewAsset {
        AssetId id;
        AssetHash hash;
    };
    std::vector<NewAsset> newAssets;
    struct Trade : public TradeAmount {
        AssetId assetId;
    };
    std::vector<Trade> trades;
};

}

#pragma once
#include "api/types/shared.hpp"
#include "data/state_updater.hpp"

struct WartBalance : public api::FundsBalance {
    static auto get_data(const DataRetrievalContext& ctx, auto callback)
    {
        return [&ctx, callback = std::move(callback)]() {
            WartBalance bal { ctx.get_wart_balance() };
            callback(std::optional<WartBalance> { bal });
        };
    }
};
struct TokenBalance1 : public api::FundsBalance {
    static auto get_data(const DataRetrievalContext& ctx, auto callback, api::TokenSpec token)
    {
        return [&ctx, callback = std::move(callback), token]() {
            auto bal { ctx.get_balance(token) };
            callback(std::optional<TokenBalance1> { bal });
        };
    }
};
struct TokenBalance2 : public api::FundsBalance {
    static auto get_data(const DataRetrievalContext& ctx, auto callback, api::TokenSpec token)
    {
        return [&ctx, callback = std::move(callback), token]() {
            auto bal { ctx.get_balance(token) };
            callback(std::optional<TokenBalance2> { bal });
        };
    }
};

struct TokenCompletion : public api_types::TokenList {
    static auto get_data(const DataRetrievalContext& ctx, auto callback, std::string namePrefix, std::string hashPrefix)
    {
        return [&ctx, callback = std::move(callback), namePrefix = std::move(namePrefix), hashPrefix = std::move(hashPrefix)]() {
            auto bal { ctx.endpoint.token_complete(namePrefix, hashPrefix) };
            callback(std::optional<TokenCompletion>(bal));
        };
    }
};

struct DataInterface {
private:
    using updater_t = DataStateUpdater<WartBalance, TokenBalance1, TokenBalance2, TokenCompletion>;
    using defer_t = std::function<void(std::function<void()>)>;
    std::shared_ptr<updater_t> updater;
    defer_t defer;
    DataRetrievalContext retrievalCtx;
    std::optional<api::TokenSpec> prevToken1;
    std::optional<api::TokenSpec> prevToken2;

public:
    StateUpdaterContext ctx() { return { retrievalCtx }; }
    DataInterface(DataRetrievalContext init, defer_t defer)
        : updater(std::make_shared<updater_t>())
        , defer(std::move(defer))
        , retrievalCtx(init)
    {
    }
    auto& retrieval_context() const { return retrievalCtx; }
    [[nodiscard]] auto& get_wart_balance(auto onComplete)
    {
        return updater->get<WartBalance>(ctx(), false, defer, std::move(onComplete));
    }
    [[nodiscard]] auto& token_balance(bool clearCache, auto onComplete, api::TokenSpec token)
    {
        bool newToken { prevToken1 != token };
        prevToken1 = token;
        return updater->get<TokenBalance1>(ctx(), newToken || clearCache, defer, std::move(onComplete), token);
    }
    [[nodiscard]] auto& token_balance2(bool clearCache, auto onComplete, api::TokenSpec token)
    {
        bool newToken { prevToken2 != token };
        prevToken2 = token;
        return updater->get<TokenBalance2>(ctx(), newToken || clearCache, defer, std::move(onComplete), token);
    }
    [[nodiscard]] auto& asset_balance(bool clearCache, auto onComplete, AssetHash asset)
    {
        return token_balance(clearCache, std::move(onComplete), api::TokenSpec::Asset(asset));
    }
    [[nodiscard]] auto& liquidity_balance(bool clearCache, auto onComplete, AssetHash asset)
    {
        return token_balance2(clearCache, std::move(onComplete), api::TokenSpec::Liquidity(asset));
    }
    [[nodiscard]] auto& token_complete(bool clearCache, auto onComplete, std::string namePrefix, std::string hashPrefix)
    {
        return updater->get<TokenCompletion>(ctx(), clearCache, defer, std::move(onComplete), namePrefix, hashPrefix);
    }
};

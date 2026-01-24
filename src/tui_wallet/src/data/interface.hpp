#pragma once
#include "api/types/shared.hpp"
#include "data/state_updater.hpp"

struct WartBalance : public api::FundsBalance {
    static std::jthread get_data(const DataRetrievalContext& ctx, auto callback)
    {
        return std::jthread([&ctx, callback = std::move(callback)]() {
            WartBalance bal { ctx.get_wart_balance() };
            callback(std::optional<WartBalance> { bal });
        });
    }
};

struct TokenCompletion : public api_types::TokenList {
    static std::jthread get_data(const DataRetrievalContext& ctx, auto callback, std::string namePrefix, std::string hashPrefix)
    {
        return std::jthread([&ctx, callback = std::move(callback), namePrefix = std::move(namePrefix), hashPrefix = std::move(hashPrefix)]() {
            auto bal { ctx.endpoint.token_complete(namePrefix, hashPrefix) };
            callback(std::optional<TokenCompletion>(bal));
        });
    }
};

struct DataInterface : public DataStateUpdater<WartBalance, TokenCompletion> {
    auto get_wart_balance(auto onComplete)
    {
        return get<WartBalance>(false, std::move(onComplete));
    }
    [[nodiscard]] auto token_complete(bool clearCache, auto onComplete, std::string namePrefix, std::string hashPrefix)
    {
        return get<TokenCompletion>(clearCache, std::move(onComplete), namePrefix, hashPrefix);
    }
};

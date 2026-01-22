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
    static std::jthread get_data(const DataRetrievalContext& ctx, auto callback, std::string s)
    {
        return std::jthread([&ctx, callback = std::move(callback), s]() {
            // endpoint.token_complete(prefix)
            auto bal { ctx.endpoint.token_complete(s) };
            callback(std::optional<TokenCompletion>(bal));
        });
    }
};

struct DataInterface : public DataStateUpdater<WartBalance, TokenCompletion> {
    auto get_wart_balance(auto onComplete)
    {
        return get<WartBalance>(false,std::move(onComplete));
    }
    [[nodiscard]] auto token_complete(bool clearCache, auto onComplete, std::string prefix)
    {
        return get<TokenCompletion>(clearCache, std::move(onComplete), prefix);
    }
};

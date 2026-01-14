#pragma once
#include "api/endpoint.hpp"
#include "wallet.hpp"
struct DataRetrievalContext {
    Endpoint endpoint;
    Wallet wallet;
    auto address() const { return wallet.address.to_string(); }
    auto get_balance(api::TokenIdOrSpec token) const
    {
        return endpoint.get_balance(address(), token);
    }
    auto get_wart_balance() const
    {
        return endpoint.get_wart_balance(address());
    }
};

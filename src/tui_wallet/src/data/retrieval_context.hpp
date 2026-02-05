#pragma once
#include "api/endpoint.hpp"
#include "communication/create_transaction.hpp"
#include "wallet.hpp"
struct DataRetrievalContext {
    Endpoint endpoint;
    Wallet wallet;
    auto address() const { return wallet.address.to_string(); }
    [[nodiscard]] TransactionCreateContext tx_create_context(NonceId nonceId, CompactUInt compactFee) const
    {
        auto [pinHeight, pinHash] { endpoint.get_pin() };
        return TransactionCreateContext {
            .pinHash { pinHash },
            .pinHeight { pinHeight },
            .nonceId { nonceId },
            .compactFee { compactFee },
            .pk { wallet.privKey }
        };
    }

public:

    auto get_balance(api::TokenIdOrSpec token) const
    {
        return endpoint.get_balance(address(), token);
    }
    auto get_wart_balance() const
    {
        return endpoint.wart_balance(address());
    }
};

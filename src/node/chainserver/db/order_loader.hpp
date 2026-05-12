#pragma once
#include "chainserver/db/types_fwd.hpp"
#include "crypto/hash.hpp"
#include "db/sqlite_fwd.hpp"
#include "defi/order.hpp"

class OrderLoaderBase {
    friend chain_db::ChainDB;
    OrderLoaderBase(sqlite::Statement& stmt);

public:
    [[nodiscard]] std::optional<OrderData> operator()() const;
    OrderLoaderBase(const OrderLoaderBase&) = delete;
    OrderLoaderBase& operator=(const OrderLoaderBase&) = delete;
    OrderLoaderBase(OrderLoaderBase&& other);
    ~OrderLoaderBase();

private:
    sqlite::Statement* stmt;
    std::optional<OrderData> loaded;
};

struct OrderDataWithTxhash: public OrderData {
    TxHash txHash;
};
template <bool ASCENDING>
class OrderLoader : public OrderLoaderBase {
    using OrderLoaderBase::OrderLoaderBase;
};

using OrderLoaderAscending = OrderLoader<true>;
using OrderLoaderDescending = OrderLoader<false>;

class OrderLoaderTxhashBase {
    friend chain_db::ChainDB;
    OrderLoaderTxhashBase(sqlite::Statement& stmt);

public:
    [[nodiscard]] std::optional<OrderDataWithTxhash> operator()() const;
    OrderLoaderTxhashBase(const OrderLoaderTxhashBase&) = delete;
    OrderLoaderTxhashBase& operator=(const OrderLoaderTxhashBase&) = delete;
    OrderLoaderTxhashBase(OrderLoaderTxhashBase&& other);
    ~OrderLoaderTxhashBase();

private:
    sqlite::Statement* stmt;
    std::optional<OrderDataWithTxhash> loaded;
};

template <bool ASCENDING>
class OrderLoaderTxhash : public OrderLoaderTxhashBase {
    using OrderLoaderTxhashBase::OrderLoaderTxhashBase;
};

using OrderLoaderTxhashAscending = OrderLoaderTxhash<true>;
using OrderLoaderTxhashDescending = OrderLoaderTxhash<false>;

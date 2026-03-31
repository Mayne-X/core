#include "glaze_convert.hpp"
#include "api/types/all.hpp"
#include "crypto/hash.hpp"
#include "general/funds.hpp"
#include "general/hex.hpp"
#include "transport/helpers/peer_addr.hpp"

namespace api {
namespace glaze {

TransactionId from(const ::TransactionId txid)
{
    return {
        .accountId = from(txid.accountId),
        .nonceId = from(txid.nonceId),
        .pinHeight = from(txid.pinHeight),
    };
}
uint32_t from(const TokenDecimals& d)
{
    return d.value();
}
AssetBasic from(const ::AssetBasic& a)
{
    return {
        .hash { serialize_hex(a.hash) },
        .id = from(a.id),
        .name { a.name.to_string() },
        .decimals = from(a.decimals)
    };
}
TransactionSignedCommon from(const api::block::TransactionSignedData& d)
{
    return {
        .originId = from(d.originId),
        .originAddress { serialize_hex(d.originAddress) },
        .fee { from(d.fee) },
        .nonceId = from(d.nonceId),
        .pinHeight = from(d.pinHeight),
    };
}
static Price make_price(Price_uint64 p, TokenDecimals d)
{
    return {
        .precExponent10 = p.base10_decimals_exponent(d),
        .exponent2 = p.mantissa_exponent2(),
        .mantissa = p.mantissa_16bit(),
        .hex { serialize_hex(to_bytes(p)) },
        .doubleAdjusted = p.to_double_adjusted(d),
        .doubleRaw = p.to_double_raw(),
    };
}
uint64_t from(const IsUint64& i)
{
    return i.value();
}

uint32_t from(const IsUint32& i)
{
    return i.value();
}

template <typename T>
static auto from(const wrt::optional<T>& o)
{
    std::optional<target_type<T>> out;
    if (o)
        out = from(*o);
    return out;
}

template <typename T>
static auto from(const std::vector<T>& v)
{
    std::vector<target_type<T>> out;
    for (auto& e : v) {
        out.push_back(from(e));
    }
    return out;
}

namespace {

FundsDecimalNoDecimals convert_funds_no_decimals(::FundsDecimal fd)
{
    return {
        .str = fd.to_string(),
        .u64 = fd.funds.value()
    };
}
FundsDecimal from(::FundsDecimal fd)
{
    return {
        .str = fd.to_string(),
        .u64 = fd.funds.value(),
        .decimals = fd.decimals.value()
    };
}
std::string convert(const ::Peeraddr a)
{
    return a.to_string();
}
FundsBalance from(const ::api::FundsBalance& fb)
{
    return {
        .total = from(fb.total),
        .locked = from(fb.locked),
        .mempool = from(fb.mempool)
    };
}
}
static BaseQuote make_base_quote(const defi::BaseQuote& bq, TokenDecimals d)
{
    return {
        .base { from(::FundsDecimal(bq.base(), d)) },
        .quote = from(bq.quote())
    };
}

Grid from(const ::Grid& g)
{
    Grid out;
    for (auto header : g) {
        out.headers.push_back(serialize_hex(header));
    }
    return out;
}

Wart from(const ::Wart w)
{
    return {
        .str { w.to_string() },
        .E8 = w.E8(),
    };
}

Hash from(const ::Hash& h)
{
    return { .hash = serialize_hex(h) };
}

BanEntry from(const ::PeerDB::BanEntry& e)
{
    return {
        .ip = e.ip.to_string(),
        .banuntil = e.banuntil,
        .reason = e.offense.err_name()
    };
}
Account from(const ::api::Account& a)
{
    return {
        .address = a.address.to_string(),
        .accountId = a.id.value()
    };
}
ThrottledPeer from(const ::api::ThrottledPeer& p)
{
    using namespace std::chrono;
    return {
        .throttle {
            .delay = int(duration_cast<seconds>(p.throttle.delay).count()),
            .blockRequest {
                .h1 = p.throttle.blockreq.h0.value(),
                .h2 = p.throttle.blockreq.h1.value(),
                .window = p.throttle.batchreq.window },
            .headerRequest {
                .h1 = p.throttle.batchreq.h0.value(),
                .h2 = p.throttle.batchreq.h1.value(),
                .window = p.throttle.batchreq.window },
        },
        .connection {
            .endpoint = p.endpoint.to_string(), .id = p.id }
    };
}
Token from(const ::api::Token& t)
{
    return {
        .id = t.id.value(),
        .spec = t.spec.to_string(),
        .name = t.name,
        .decimals = t.token_decimals().value(),
    };
}
AssetDetail from(const ::AssetDetail& ad)
{
    return {
        .hash = serialize_hex(ad.hash),
        .id = ad.id.value(),
        .name = ad.name.to_string(),
        .decimals = ad.decimals.value(),
        .height = from(ad.height),
        .ownerAccountId = ad.ownerAccountId.value(),
        .totalSupply = from(::FundsDecimal(ad.totalSupply, ad.decimals)),
        .groupId = ad.group_id.value(),
        .parentId = from(ad.parent_id),
    };
}
AssetLookupTrace from(const ::api::AssetLookupTrace& a)
{
    return {
        .fails = from(a.fails),
        .snapshotHeight = from(a.snapshotHeight)
    };
}
TokenBalanceLookup from(const ::api::TokenBalanceLookup& l)
{
    TokenBalanceLookup out {
        .balance = from(l.balance),
        .token = from(l.token),
        .lookupTrace = from(l.lookupTrace),
        .account = from(l.account),
    };
    return out;
}
TransactionDetails to_json(const api::TransactionDetails& d)
{
    wrt::Overload convert_transaction(
        [&](const api::MinedReward& m) -> reward::Transaction {
            return {
                .data {
                    .toAddress { m.transaction.data.toAddress.to_string() },
                    .amount = from(m.transaction.data.amount),
                },
                .hash = serialize_hex(m.transaction.hash),
            };
        },
        [&](const api::MaybeMinedWartTransfer& m) -> wart_transfer::Transaction {
            return {
                .data {
                    .toAddress { m.transaction.data.toAddress.to_string() },
                    .amount = from(m.transaction.data.amount),
                },
                .hash { serialize_hex(m.transaction.hash) },
                .signedCommon { from(m.transaction.signedData) }
            };
        },
        [&](const api::MaybeMinedTokenTransfer& m) -> token_transfer::Transaction {
            auto& data { m.transaction.data };
            auto funds { ::FundsDecimal(data.amount, data.assetInfo.decimals) };
            return {
                .data {
                    .toAddress { data.toAddress.to_string() },
                    .amount = from(funds),
                    .asset = from(data.assetInfo),
                    .isLiquidity = data.isLiquidity,
                    .tokenSpec = TokenSpec(data.assetInfo.hash, data.isLiquidity).to_string(),
                },
                .hash { serialize_hex(m.transaction.hash) },
                .signedCommon { from(m.transaction.signedData) }
            };
        },
        [&](const api::MaybeMinedAssetCreation& m) -> asset_creation::TransactionMaybeProcessed {
            asset_creation::TransactionMaybeProcessed out {
                .data { .name = m.transaction.data.name.to_string(),
                    .supply = from(m.transaction.data.supply) },
                .processed {},
                .hash { serialize_hex(m.transaction.hash) },
                .signedCommon { from(m.transaction.signedData) }
            };

            auto& aid { m.transaction.data.assetId };
            if (aid) {
                out.processed = asset_creation::Processed { from(*aid) };
            }
            return out;
        },
        [&](const api::MaybeMinedNewOrder& m) -> new_order::TransactionMaybeProcessed {
            auto& d { m.transaction.data };
            // d.amount_decimal
            new_order::TransactionMaybeProcessed out {
                .data {
                    .baseAsset { from(d.assetInfo) },
                    .amount { from(d.amount_decimal()) },
                    .limit { make_price(d.limit, d.assetInfo.decimals) },
                    .buy = d.buy },
                .processed {},
                .hash { serialize_hex(m.transaction.hash) },
                .signedCommon { from(m.transaction.signedData) }
            };
            if (d.filled) {
                out.processed = new_order::Processed {
                    .filled = from(::FundsDecimal(*d.filled, d.assetInfo.decimals))
                };
            }
            return out;
        },
        [&](const api::MinedMatch& m) -> match::Transaction {
            auto& d { m.transaction.data };
            std::vector<match::Data::SwapEntry> buySwaps, sellSwaps;
            for (auto& s : m.transaction.data.buySwaps) {
                defi::BaseQuote bq { s.base(), s.quote() };
                buySwaps.push_back(
                    { .swapped = make_base_quote(bq, d.assetInfo.decimals),
                        .historyId = from(s.referred_history_id()) });
            }
            for (auto& s : m.transaction.data.sellSwaps) {
                defi::BaseQuote bq { s.base(), s.quote() };
                sellSwaps.push_back(
                    { .swapped = make_base_quote(bq, d.assetInfo.decimals),
                        .historyId = from(s.referred_history_id()) });
            }
            // std::vector<SwapEntry> sellSwaps;
            return {
                .data {
                    .baseAsset { from(d.assetInfo) },
                    .poolBefore { make_base_quote(d.poolBefore, d.assetInfo.decimals) },
                    .poolAfter { make_base_quote(d.poolBefore, d.assetInfo.decimals) },
                    .buySwaps { std::move(buySwaps) },
                    .sellSwaps { std::move(sellSwaps) } },

                .hash { serialize_hex(m.transaction.hash) },
            };
        },
        [&](const api::MaybeMinedLiquidityDeposit& m) -> liquidity_deposit::TransactionMaybeProcessed {
            auto& d { m.transaction.data };
            defi::BaseQuote bq { d.baseDeposited, d.quoteDeposited };
            liquidity_deposit::TransactionMaybeProcessed out {
                .data {
                    .baseAsset { from(d.assetInfo) },
                    .deposited { make_base_quote(bq, d.assetInfo.decimals) } },
                .processed {},
                .hash { serialize_hex(m.transaction.hash) },
                .signedCommon { from(m.transaction.signedData) }
            };
            if (d.sharesReceived) {
                out.processed = liquidity_deposit::Processed { .sharesReceived = from(
                                                                   ::FundsDecimal(*d.sharesReceived, d.assetInfo.decimals)) };
            }
            return out;
        },
        [&](const api::MaybeMinedLiquidityWithdrawal& m) -> liquidity_withdrawal::TransactionMaybeProcessed {
            auto& d { m.transaction.data };
            ::FundsDecimal fd(d.sharesRedeemed, TokenDecimals::LIQUIDITY);
            liquidity_withdrawal::TransactionMaybeProcessed out {
                .data {
                    .baseAsset { from(d.assetInfo) },
                    .sharesRedeemed { from(fd) } },
                .processed {},
                .hash { serialize_hex(m.transaction.hash) },
                .signedCommon { from(m.transaction.signedData) }
            };
            if (d.received) {
                out.processed = liquidity_withdrawal::Processed(make_base_quote(*d.received, d.assetInfo.decimals));
            }
            return out;
        },
        [&](const api::MaybeMinedCancelation& m) -> cancelation::TransactionMaybeProcessed {
            auto& d { m.transaction.data };
            // ::FundsDecimal fd(d.sharesRedeemed, TokenDecimals::LIQUIDITY);
            cancelation::TransactionMaybeProcessed out {
                .data { .cancelTxid = from(m.transaction.data.cancelTxid) },
                .processed {},
                .hash { serialize_hex(m.transaction.hash) },
                .signedCommon { from(m.transaction.signedData) }
            };
            if (d.canceledOrder) {
                auto& o { *d.canceledOrder };
                out.processed = cancelation::Processed {
                    .baseAsset = from(o.assetInfo),
                    .buy = o.buy,
                    .historyId = from(o.historyId),
                    .remaining = from(::FundsDecimal(o.remaining, o.assetInfo.decimals))
                };
            }
            return out;
        });
    auto mined_transaction_details { [&](const TransactionMinedData* md, uint32_t confirmations, auto&& arg) -> TransactionDetails {
        if (md) {
            auto& m { *md };
            return TransactionDetails {
                .transaction { convert_transaction(std::forward<decltype(arg)>(arg)) },
                .type { arg.transaction.data.label },
                .mined { TransactionDetails::Mined {
                    .historyId = from(m.hid),
                    .block {
                        .hegiht = m.block.height.value(),
                        .hash = serialize_hex(m.block.hash),
                        .timestamp = m.block.timestamp,
                    },
                } },
                .confirmations = confirmations,
            };
        } else {
            return TransactionDetails {
                .transaction { convert_transaction(std::forward<decltype(arg)>(arg)) },
                .type { arg.transaction.data.label },
                .mined {},
                .confirmations = confirmations,
            };
        }
    } };
    wrt::Overload convert([&]<typename T>(const api::MaybeMined<T>& a) {
        if (a.mined) 
            return mined_transaction_details(&*a.mined,a.confirmations,a);
        else 
            return mined_transaction_details(nullptr,a.confirmations,a); },
        [&]<typename T>(const api::Mined<T>& a) {
            return mined_transaction_details(&a.mined, a.confirmations, a);
        });
    return d.visit(convert);
}
}
}

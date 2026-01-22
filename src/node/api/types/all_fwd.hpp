#pragma once
#include "api/types/shared_fwd.hpp"
#include "wrt/variant_fwd.hpp"
namespace api {
struct AccountHistory;
struct AccountIdOrAddress;
struct AddressCount;
struct Account;
struct AssetLookupTrace;
struct Block;
struct BlockBinary;
struct AssetSearchArgs;
struct AssetSearchResult;
struct ChainHead;
struct CompleteBlock;
struct HashrateBlockChart;
struct HashrateChartRequest;
struct HashrateInfo;
struct HashrateTimeChart;
struct Head;
struct HeaderInfo;
struct HeightOrHash;
struct MempoolEntries;
struct MempoolUpdate;
struct MiningState;
struct Peerinfo;
struct ParsedPrice;
struct PeerinfoConnections;
struct Raw;
struct TokenBalanceLookup;
struct WartBalance;
struct JanushashNumber;
template <typename TxType>
struct Temporal;
using RewardTransaction = Temporal<block::Reward>;
using WartTransferTransaction = Temporal<block::WartTransfer>;
using TokenTransferTransaction = Temporal<block::TokenTransfer>;
using AssetCreationTransaction = Temporal<block::AssetCreation>;
using NewOrderTransaction = Temporal<block::NewOrder>;
using MatchTransaction = Temporal<block::Match>;
using LiquidityDepositTransaction = Temporal<block::LiquidityDeposit>;
using LiquidityWithdrawalTransaction = Temporal<block::LiquidityWithdrawal>;
using CancelationTransaction = Temporal<block::TransactionCancelation>;
using OrderCancelationTransaction = Temporal<block::OrderCancelation>;

using Transaction = wrt::variant<
    RewardTransaction,
    WartTransferTransaction,
    TokenTransferTransaction,
    AssetCreationTransaction,
    NewOrderTransaction,
    MatchTransaction,
    LiquidityDepositTransaction,
    LiquidityWithdrawalTransaction,
    CancelationTransaction,
    OrderCancelationTransaction>;

struct Richlist;
struct RichlistInfo;
struct Rollback;
struct Round16Bit;
struct TransactionsByBlocks;
struct TransactionMinfee;
struct Token;
struct Wallet;
struct DBSize;
struct NodeInfo;
struct IPCounter;
struct ThrottleState;
struct ThrottledPeer;
}

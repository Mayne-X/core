#pragma once
namespace api {
namespace block {

struct RewardData;
struct WartTransferData;
struct TokenTransferData;
struct AssetCreationData;
struct NewOrderData;
struct LiquidityDepositData;
struct LiquidityWithdrawalData;
struct CancelationData;
struct OrderCancelationData;
struct MatchData;

template <typename T>
struct WithHistoryBase;
template <typename T>
struct WithSignedInfo;

using Reward = WithHistoryBase<RewardData>;
using WartTransfer = WithSignedInfo<WartTransferData>;
using TokenTransfer = WithSignedInfo<TokenTransferData>;
using AssetCreation = WithSignedInfo<AssetCreationData>;
using NewOrder = WithSignedInfo<NewOrderData>;
using LiquidityDeposit = WithSignedInfo<LiquidityDepositData>;
using LiquidityWithdrawal = WithSignedInfo<LiquidityWithdrawalData>;
using TransactionCancelation = WithSignedInfo<CancelationData>;
using OrderCancelation = WithHistoryBase<OrderCancelationData>;
using Match = WithHistoryBase<MatchData>;

}
}

#pragma once
namespace api {
namespace block {

struct RewardData;
struct WartTransferData;
struct TokenTransferData;
struct AssetCreationData;
struct LimitSwapData;
struct LiquidityDepositData;
struct LiquidityWithdrawalData;
struct CancelationData;
struct MatchData;
struct TransactionAddResult;

template <typename T>
struct IsTransaction;
template <typename T>
struct IsSignedTransaction;
template <typename T>
struct WithHistoryId;

using Reward = IsTransaction<RewardData>;
using WartTransfer = IsSignedTransaction<WartTransferData>;
using TokenTransfer = IsSignedTransaction<TokenTransferData>;
using AssetCreation = IsSignedTransaction<AssetCreationData>;
using LimitSwap = IsSignedTransaction<LimitSwapData>;
using LiquidityDeposit = IsSignedTransaction<LiquidityDepositData>;
using LiquidityWithdrawal = IsSignedTransaction<LiquidityWithdrawalData>;
using Cancelation = IsSignedTransaction<CancelationData>;
using Match = IsTransaction<MatchData>;

struct MempoolEntry;

}
}

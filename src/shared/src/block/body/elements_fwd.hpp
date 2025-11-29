#pragma once
#include "block/body/labels.hpp"
#include "general/base_elements_fwd.hpp"
#include "general/structured_reader_fwd.hpp"
namespace block {
namespace body {

template <typename... Ts>
struct Combined;
template <typename T>
struct HookMerkle;
template <typename... Ts>
struct SignedCombined;
template <StaticString tag, typename... Ts>
using TaggedSignedCombined = Tag<tag, SignedCombined<Ts...>>;

using Reward = HookMerkle<Combined<ToAccIdEl, WartEl>>;
using WartTransfer = HookMerkle<TaggedSignedCombined<::block::labels::wartTransfer, ToAccIdEl, WartEl>>;
using AssetTransfer = HookMerkle<TaggedSignedCombined<::block::labels::assetTransfer, ToAccIdEl, NonzeroAmountEl>>;
using LiquidityTransfer = HookMerkle<TaggedSignedCombined<::block::labels::liquidityTransfer, ToAccIdEl, NonzeroSharesEl>>;
using AssetCreation = HookMerkle<TaggedSignedCombined<::block::labels::assetCreation, AssetSupplyEl, AssetNameEl>>;
using Order = HookMerkle<TaggedSignedCombined<::block::labels::limitSwap, BuyEl, NonzeroAmountEl, LimitPriceEl>>;
struct CancelationBase;
using Cancelation = HookMerkle<Tag<::block::labels::cancelation, CancelationBase>>;
using LiquidityDeposit = HookMerkle<TaggedSignedCombined<::block::labels::liquidityDeposit, BaseEl, QuoteEl>>;
using LiquidityWithdrawal = HookMerkle<TaggedSignedCombined<"liquidityWithdrawal", NonzeroAmountEl>>;
}
}

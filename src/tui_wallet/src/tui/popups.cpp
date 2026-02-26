#include "popups.hpp"
#include "general/funds.hpp"
#include "global.hpp"
#include "tabs.hpp"
#include "transaction.hpp"
#include "validators.hpp"
namespace ui {
namespace {

auto format_token_caption(api::TokenSpec spec, std::string name)
{
    return std::format("{} ({})", spec.to_string(), name);
}

}

void TransferPopup::on_cancel() { closed = true; }

// auto work {
//     [] -> NotificationData {
//         return { "Hello", "world" };
//     }
// };

void TransferPopup::on_create()
{
    // Parse text fields
    auto nId { try_parse<uint32_t>(editNonceId.get()->content) };
    auto amt { Funds_uint64::parse(editAmount.get()->content, token.prec()).and_then([](Funds_uint64 f) { return f.nonzero(); }) };
    auto toAddress { Address::parse(editToAddr.get()->content) };

    // Wart itself has no liquidity
    if (token.spec.assetHash.is_wart() && token.spec.isLiquidity)
        return; // this should not happen and is not allowed

    if (!nId || !fee || !amt || !toAddress)
        return;

    auto properties { KVProperties {
        .title { "Transfer" },
        .entries {
            { "Token: ", token.to_string() },
            { "Amount (" + token.pretty_name() + "): ", amt->to_decimal(token.prec()).to_string() },
            { "Destination: ", toAddress->to_string() },
            { "Fee (WART): ", fee.value().to_string() },
            { "NonceId: ", std::to_string(*nId) },
        } } };
    if (token.spec.isLiquidity) {
        properties.entries.push_back(
            { "NOTE: ",
                "This transfer is for pool liquidity, not the actual asset!" });
    }

    auto work {
        [token = token, nonceId = NonceId(*nId), compactFee = fee.value(), amount = *amt, toAddr = *toAddress] -> NotificationData {
            auto& drc { global::data_interface().retrieval_context() };
            auto ctx { drc.tx_create_context(nonceId, compactFee) };
            auto& s { token.spec };
            auto tx { [&] -> std::string {
                if (s.assetHash.is_wart()) {
                    return WartTransferCreate(ctx, toAddr, amount.as_wart());
                } else {
                    return TokenTransferCreate(ctx, s.assetHash, s.isLiquidity, toAddr, amount);
                }
            }() };
            auto hash { drc.endpoint.send_transaction(tx) };
            return { "Success", std::format("Transaction was sent.\r Transaction Hash: {}", serialize_hex(hash)) };
        }
    };
    make_popup<ConfirmationPopup>(std::move(properties), work, close_callback());
};

TransferPopup::TransferPopup(GUI& gui, TokenInfo token)
    : GUIComponent(gui)
    , token(std::move(token))
    , amount(token.prec())
    , editAmount(ui::LabeledValidated("Amount:  ", amount.validator()))
    , editToAddr(ui::LabeledValidated("Destination: ", validate_address))
    , editFee(ui::LabeledValidated("Fee (WART): ", fee.validator()))
    , editNonceId(ui::LabeledValidated("NonceId: ", validate_nonce_id))
    , btnCancel(Button("Cancel", [&]() { this->on_cancel(); }))
    , btnCreate(Button("Create", [&]() { this->on_create(); }))
{

    editToAddr->content = "0000000000000000000000000000000000000000de47c9b2";
    editToAddr->validate();
    Add(Container::Vertical({ editToAddr, editAmount, editFee, editNonceId,
        Container::Horizontal({ btnCancel, btnCreate }) }));
}

CreatePopup::CreatePopup(GUI& gui, AssetName name)
    : GUIComponent(gui)
    , assetName()
    , editName(ui::LabeledValidated("Asset Name:  ", assetName.validator()))
    , editSupply(ui::LabeledValidated("Asset Supply:  ", assetSupply.validator()))
    , editFee(ui::LabeledValidated("Fee (WART):  ", fee.validator()))
    , editNonceId(ui::LabeledValidated("NonceId: ", nonce.validator()))
    , btnCancel(Button("Cancel", [&]() { this->on_cancel(); }))
    , btnCreate(Button("Create", [&]() { this->on_create(); }))
{
    editName->content = name.to_string();
    editName->validate();
    Add(Container::Vertical({ editName, editSupply, editFee, editNonceId,
        Container::Horizontal({ btnCancel, btnCreate }) }));
}

void CreatePopup::on_create()
{
    if (!assetName || !assetSupply || !fee || !nonce)
        return;

    auto properties { KVProperties {
        .title { "Asset Creation" },
        .entries {
            { "Asset Name: ", assetName.value().to_string() },
            { "Total Supply: ", assetSupply.value().to_string() },
            { "Precision: ", assetSupply.value().precision.to_string() },
            { "Fee (WART): ", fee.value().to_string() },
            { "NonceId: ", nonce.value().to_string() },
        } } };
    auto work {
        [assetName = assetName.value(),
            assetSupply = assetSupply.value(),
            compactFee = fee.value(),
            nonceId = nonce.value()]
        -> NotificationData {
            auto& drc { global::data_interface().retrieval_context() };
            auto ctx { drc.tx_create_context(nonceId, compactFee) };
            auto hash { [&] {
                AssetCreationCreate tx(ctx, assetSupply, assetName);
                return drc.endpoint.send_transaction(tx);
            }() };
            return { "Success", std::format("Transaction was sent.\r Transaction Hash: {}", serialize_hex(hash)) };
        }
    };
    make_popup<ConfirmationPopup>(std::move(properties), work, close_callback());
};

void CreatePopup::on_cancel() { closed = true; }
Element CreatePopup::OnRender()
{
    auto prec { assetSupply.has_value() ? assetSupply.value().precision : TokenPrecision(0) };
    return vbox(
        { window(text("Create Asset"),
              vbox({ editName->Render(),
                  editSupply->Render(),
                  text("Decimals: " + std::to_string(int(prec.value()))),
                  editFee->Render(), editNonceId->Render() })),
            hbox(btnCancel, btnCreate->Render()) | center });
}

void SwapPopup::on_create()
{
    auto allValid { editAmount->valid && editLimit->valid && editFee->valid };
    if (!allValid)
        return;
    auto assetCaption { format_token_caption(api::TokenSpec(asset.hash, false), asset.name) };
    auto wartCaption { format_token_caption(api::TokenSpec::WART, "Wart") };

    auto nId { try_parse<uint32_t>(editNonceId.get()->content) };
    auto l { Price_uint64::from_string_adjusted(editLimit.get()->content, asset.precision) };

    if (!nId || !fee || !amount.has_value() || !l)
        return;

    auto properties { KVProperties {
        .title { "Swap" },
        .entries {
            { "From Token: ",
                is_buy() ? wartCaption : assetCaption },
            { "To Token: ",
                is_buy() ? assetCaption : wartCaption },
            { "Amount: ", amount.value().to_decimal(asset.precision).to_string() },
            { "Limit Price: ", std::to_string(l->to_double_adjusted(asset.precision)) },
            { "Fee (WART): ", fee.value().to_string() } } } };
    auto work {
        [asset = asset, nonceId = NonceId(*nId), isBuy = is_buy(), compactFee = fee.value(), amount = amount.value(), limit = *l] -> NotificationData {
            auto& drc { global::data_interface().retrieval_context() };
            auto ctx { drc.tx_create_context(nonceId, compactFee) };
            auto& s { asset };
            auto hash { [&] {
                // DEFINE_CREATE_MESSAGE(LimitSwapCreate, ::block::labels::limitSwap, AssetHashEl, BuyEl, NonzeroAmountEl, LimitPriceEl)
                LimitSwapCreate tx(ctx, s.hash, isBuy, amount, limit);
                return drc.endpoint.send_transaction(tx);
            }() };
            return { "Success", std::format("Transaction was sent.\r Transaction Hash: {}", serialize_hex(hash)) };
        }
    };
    make_popup<ConfirmationPopup>(std::move(properties), work, close_callback());
};

void SwapPopup::on_cancel() { closed = true; }
Element SwapPopup::OnRender()
{
    this->amount.set_prec(is_buy() ? TokenPrecision::WART : asset.precision);
    editAmount->validate();
    editLimit->set_validator(LimitValidator(asset.precision, !is_buy()));
    editAmount->label = std::string("Amount (") + (is_buy() ? "WART" : asset.name) + "): ";
    editLimit->label = "Limit (" + std::string(is_buy() ? "MAX" : "MIN") + " Price): ";
    return vbox(
        { window(text("New Swap"),
              vbox({ text("Base Asset: " + asset.to_string()),
                  hbox(text("Swap direction: "), toggle->Render()),
                  editAmount->Render(), editLimit->Render(), editFee->Render(), editNonceId->Render() })),
            hbox(btnCancel, btnCreate->Render()) | center });
}

SwapPopup::SwapPopup(GUI& gui, AssetInfo a, bool buy)
    : GUIComponent(gui)
    , asset(std::move(a))
    , swap_directions { "BUY " + asset.name + " WITH WART",
        "SELL " + asset.name + " FOR WART" }
    , side_selected(buy ? 0 : 1)
    , amount(is_buy() ? asset.precision : TokenPrecision::WART)
    , editAmount(ui::LabeledValidated("Amount:  ", amount.validator()))
    , editLimit(ui::LabeledValidated("Limit Price:  "))
    , editFee(ui::LabeledValidated("Fee (WART):  ", fee.validator()))
    , editNonceId(ui::LabeledValidated("NonceId: ", validate_nonce_id))
    , toggle(Toggle(swap_directions, &side_selected))
    , btnCancel(Button("Cancel", [&]() { this->on_cancel(); }))
    , btnCreate(Button("Create", [&]() { this->on_create(); }))
{
    Add(Container::Vertical({ toggle, editAmount, editLimit, editFee, editNonceId,
        Container::Horizontal({ btnCancel, btnCreate }) }));
}
void FarmPopup::on_create()
{
    auto precision { is_deposit() ? asset.precision : TokenPrecision::LIQUIDITY };
    auto nId { try_parse<uint32_t>(editNonceId.get()->content) };

    if ((is_deposit() && !wart) || !amount || !fee || !nId)
        return;

    KVProperties properties {
        .title { (is_deposit() ? "Deposit liquidity into " : "Withdraw liquidity from ") + asset.market() + " pool" },
        .entries { { "Base Asset: ", asset.to_string() },
            { "Pool: ", asset.market() } }
    };
    if (is_deposit()) {
        properties.entries.push_back({ "Amount (WART): ", wart.value().to_string() });
        properties.entries.push_back({ "Amount (" + asset.token(false).pretty_name() + "): ", amount.value().to_decimal(precision).to_string() });
    } else {
        properties.entries.push_back({ "Amount (" + asset.token(true).pretty_name() + "): ", amount.value().to_decimal(precision).to_string() });
    }
    properties.entries.push_back({ "Fee (WART): ", fee.value().to_string() });
    properties.entries.push_back({ "NonceId: ", std::to_string(*nId) });
    std::function<NotificationData()> work;
    if (is_deposit()) {
        // cannot both be zero
        if (amount.value().is_zero() && wart.value().is_zero())
            return;
        // we must create a LiquidityDepositCreate
        work = [asset = asset, nonceId = NonceId(*nId), compactFee = fee.value(), amountWart = wart.value(), amountBase = amount.value()] -> NotificationData {
            auto& drc { global::data_interface().retrieval_context() };
            auto ctx { drc.tx_create_context(nonceId, compactFee) };
            auto& s { asset };
            auto hash { [&] {
                LiquidityDepositCreate tx(ctx, s.hash, amountBase, amountWart);
                return drc.endpoint.send_transaction(tx);
            }() };
            return { "Success", std::format("Transaction was sent.\r Transaction Hash: {}", serialize_hex(hash)) };
        };
    } else {
        auto nonzeroShares { amount.value().nonzero() };
        if (!nonzeroShares)
            return;
        // we must create a LiquidityWithdrawalCreate
        work = [asset = asset, nonceId = NonceId(*nId), compactFee = fee.value(), amountShares = *nonzeroShares] -> NotificationData {
            auto& drc { global::data_interface().retrieval_context() };
            auto ctx { drc.tx_create_context(nonceId, compactFee) };
            auto& s { asset };
            auto hash { [&] {
                // DEFINE_CREATE_MESSAGE(LimitSwapCreate, ::block::labels::limitSwap, AssetHashEl, BuyEl, NonzeroAmountEl, LimitPriceEl)
                LiquidityWithdrawalCreate tx(ctx, s.hash, amountShares);
                return drc.endpoint.send_transaction(tx);
            }() };
            return { "Success", std::format("Transaction was sent.\r Transaction Hash: {}", serialize_hex(hash)) };
        };
    }
    make_popup<ConfirmationPopup>(std::move(properties), work, close_callback());
};
void FarmPopup::on_cancel() { closed = true; }

FarmPopup::FarmPopup(GUI& gui, AssetInfo a, bool deposit)
    : GUIComponent(gui)
    , asset(std::move(a))
    , liquidity_actions { "DEPOSIT LIQUIDITY",
        "WITHDRAW LIQUIDITY" }
    , side_selected(deposit ? 0 : 1)
    , amount(is_deposit() ? a.precision : TokenPrecision::LIQUIDITY)
    , editWart(ui::LabeledValidated("Amount (WART): ",
          [this](std::string& wartContent) {
              wart.validate(wartContent);
              if (amount) {
                  if (is_deposit()
                      && wart
                      && amount.value().is_zero() && wart.value().is_zero()) {
                      // if we are in deposit mode, then the restriction is that not both can be zero but one can be zero,
                      editBase->valid = false;
                      return false;
                  } else {
                      editBase->valid = true;
                  }
              }
              return wart.has_value();
          }))
    , maybeWart(Maybe(editWart, [&] { return is_deposit(); }))
    , editBase(ui::LabeledValidated("", [this](std::string& s) -> bool {
        amount.validate(s);
        if (wart) {
            if (is_deposit() && amount && amount.value().is_zero() && wart.value().is_zero()) {
                editWart->valid = false;
                return false;
            } else {
                editWart->valid = true;
            }
        }
        if (is_deposit()) {
            return amount.has_value();
        } else {
            // if we withdraw liquidity, there is only one field and it should
            // be nonzero
            return amount.has_value() && !amount.value().is_zero();
        }
    }))
    , editFee(ui::LabeledValidated("Fee (WART):  ", fee.validator()))
    , editNonceId(ui::LabeledValidated("NonceId: ", validate_nonce_id))
    , toggle([&] {
        auto opt { MenuOption::Toggle() };
        opt.on_change = [&] { editBase->validate(); };
        return Menu(liquidity_actions, &side_selected, opt);
    }())
    , btnCancel(Button("Cancel", [&]() { this->on_cancel(); }))
    , btnCreate(Button("Create", [&]() { this->on_create(); }))
{

    Add(Container::Vertical({ toggle,
        maybeWart, editBase, editFee, editNonceId,
        Container::Horizontal({ btnCancel, btnCreate }) }));
}
Element FarmPopup::OnRender()
{
    editBase->label = "Amount (" + asset.token(!is_deposit()).pretty_name() + "): ";
    return vbox(
        { window(text("Farm Liquidity"),
              vbox({ text("Base Asset: " + asset.to_string()),
                  text("Pool: " + asset.market()),
                  hbox(text("Liquidity action: "), toggle->Render()),
                  maybeWart->Render(), editBase->Render(), editFee->Render(), editNonceId->Render() })),
            hbox(btnCancel, btnCreate->Render()) | center });
}
} // namespace ui

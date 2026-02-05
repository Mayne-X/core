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

auto work {
    [] -> NotificationData {
        return { "Hello", "world" };
    }
};

void TransferPopup::on_create()
{
    auto allValid { amount->valid && toAddr->valid && fee->valid && nonceId->valid };
    if (!allValid)
        return;

    auto properties { KVProperties {
        .title { "Transfer" },
        .entries {
            { "Token ", token.to_string() },
            { "Amount (" + token.pretty_name() + ") ", amount.get()->content },
            { "Destination ", toAddr.get()->content },
            { "Fee (WART) ", fee.get()->content },
            { "NonceId ", nonceId.get()->content },
        } } };
    if (token.spec.isLiquidity) {
        properties.entries.push_back(
            { "NOTE: ",
                "This transfer is for pool liquidity, not the actual asset!" });
    }

    // Parse text fields
    auto nId { try_parse<uint32_t>(nonceId.get()->content) };
    auto compactFee { Wart::parse(fee.get()->content).transform([](Wart w) { return CompactUInt::compact(w); }) };
    auto amt { Funds_uint64::parse(amount.get()->content, token.prec()).and_then([](Funds_uint64 f) { return f.nonzero(); }) };
    auto toAddress { Address::parse(toAddr.get()->content) };

    // Wart itself has no liquidity
    if (token.spec.assetHash.is_wart() && token.spec.isLiquidity)
        return; // this should not happen and is not allowed

    if (!nId || !compactFee || !amt || !toAddress)
        return;

    auto work {
        [token = token, nonceId = NonceId(*nId), compactFee = *compactFee, amount = *amt, toAddr = *toAddress] -> NotificationData {
            auto& drc { global::data_interface().retrieval_context() };
            auto ctx { drc.tx_create_context(nonceId, compactFee) };
            auto& s { token.spec };
            auto hash { [&] {
                if (s.assetHash.is_wart()) {
                    WartTransferCreate tx(ctx, toAddr, amount.as_wart());
                    return drc.endpoint.send_transaction(tx);
                } else {
                    TokenTransferCreate tx(ctx, s.assetHash, s.isLiquidity, toAddr, amount);
                    return drc.endpoint.send_transaction(tx);
                }
            }() };
            return { "Success", std::format("Transaction was sent.\r Transaction Hash: {}", serialize_hex(hash)) };
        }
    };
    make_popup<ConfirmationPopup>(std::move(properties), work, close_callback());
};

TransferPopup::TransferPopup(GUI& gui, TokenInfo token)
    : GUIComponent(gui)
    , token(std::move(token))
    , amount(ui::LabeledValidated("Amount:  ", FundsValidator(token.prec())))
    , toAddr(ui::LabeledValidated("Destination: ", address_validator))
    , fee(ui::LabeledValidated("Fee (WART): ", wart_validator))
    , nonceId(ui::LabeledValidated("NonceId: ", nonce_id_validator))
    , btnCancel(Button("Cancel", [&]() { this->on_cancel(); }))
    , btnCreate(Button("Create", [&]() { this->on_create(); }))
{
    Add(Container::Vertical({ toAddr, amount, fee, nonceId,
        Container::Horizontal({ btnCancel, btnCreate }) }));
}

void SwapPopup::on_create()
{
    auto allValid { inputAmount->valid && inputLimit->valid && inputFee->valid };
    if (!allValid)
        return;
    auto assetCaption { format_token_caption(api::TokenSpec(asset.hash, false), asset.name) };
    auto wartCaption { format_token_caption(api::TokenSpec::WART, "Wart") };

    auto nId { try_parse<uint32_t>(inputNonceId.get()->content) };
    auto compactFee { Wart::parse(inputFee.get()->content).transform([](Wart w) { return CompactUInt::compact(w); }) };
    auto amt { Funds_uint64::parse(inputAmount.get()->content, asset.precision).and_then([](Funds_uint64 f) { return f.nonzero(); }) };
    auto l { Price_uint64::from_string_adjusted(inputLimit.get()->content, asset.precision) };

    if (!nId || !compactFee || !amt || !l)
        return;

    auto properties { KVProperties {
        .title { "Swap" },
        .entries {
            { "From Token ",
                is_buy() ? wartCaption : assetCaption },
            { "To Token ",
                is_buy() ? assetCaption : wartCaption },
            { "Amount", amt->to_decimal(asset.precision).to_string() },
            { "Limit Price ", std::to_string(l->to_double_adjusted(asset.precision)) },
        } } };
    auto work {
        [asset = asset, nonceId = NonceId(*nId), isBuy = is_buy(), compactFee = *compactFee, amount = *amt, limit = *l] -> NotificationData {
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
    inputAmount->set_validator(FundsValidator(is_buy() ? asset.precision : TokenPrecision::WART));
    inputLimit->set_validator(LimitValidator(asset.precision, !is_buy()));
    inputAmount->label = std::string("Amount (") + (is_buy() ? "WART" : asset.name) + "): ";
    inputLimit->label = "Limit (" + std::string(is_buy() ? "MAX" : "MIN") + " Price): ";
    return vbox(
        { window(text("New Swap"),
              vbox({ text("Base Asset: " + asset.to_string()),
                  hbox(text("Swap direction: "), toggle->Render()),
                  inputAmount->Render(), inputLimit->Render(), inputFee->Render() })),
            hbox(btnCancel, btnCreate->Render()) | center });
}

SwapPopup::SwapPopup(GUI& gui, AssetInfo a, bool buy)
    : GUIComponent(gui)
    , asset(std::move(a))
    , swap_directions { "BUY " + asset.name + " WITH WART",
        "SELL " + asset.name + " FOR WART" }
    , side_selected(buy ? 0 : 1)
    , inputAmount(ui::LabeledValidated("Amount:  "))
    , inputLimit(ui::LabeledValidated("Limit Price:  "))
    , inputFee(ui::LabeledValidated("Fee (WART):  ", fee_validator))
    , inputNonceId(ui::LabeledValidated("NonceId: ", nonce_id_validator))
    , toggle(Toggle(swap_directions, &side_selected))
    , btnCancel(Button("Cancel", [&]() { this->on_cancel(); }))
    , btnCreate(Button("Create", [&]() { this->on_create(); }))
{
    Add(Container::Vertical({ toggle, inputAmount, inputLimit, inputFee,
        Container::Horizontal({ btnCancel, btnCreate }) }));
}
void FarmPopup::on_create()
{
    auto allValid { inputWart->valid && inputBase->valid && inputLimit->valid && inputFee->valid };
    if (!allValid)
        return;
    auto properties { KVProperties {
        .title { "Farm" },
        .entries {
            { "From Token ",
                "95ae6efb2f4fe5e4fd3a5b21df7f755f878383610505fe64 (WART)" },
            { "To Token ",
                "95ae6efb2f4fe5e4fd3a5b21df7f755f878383610505fe64 (WART)" },
            { "Amount", inputWart.get()->content },
            { "Limit Price ", inputLimit.get()->content },
        } } };

    // onconfirm_generator_t generator {
    //     [tr = shared_from_this()](result_cb_t cb) -> std::function<void()> {
    //         return [tr = std::move(tr), cb = std::move(cb)]() mutable {
    //             std::thread t([tr = std::move(tr), cb = std::move(cb)] {
    //                 tr->closed = true;
    //                 std::this_thread::sleep_for(std::chrono::seconds(2));
    //                 cb("Success", "Transaction was sent");
    //             });
    //             t.detach();
    //         };
    //     }
    // };
    make_popup<ConfirmationPopup>(std::move(properties), work, close_callback());
};
void FarmPopup::on_cancel() { closed = true; }

FarmPopup::FarmPopup(GUI& gui, AssetInfo a, bool deposit)
    : GUIComponent(gui)
    , asset(std::move(a))
    , liquidity_actions { "DEPOSIT LIQUIDITY",
        "WITHDRAW LIQUIDITY" }
    , side_selected(deposit ? 0 : 1)
    , inputWart(ui::LabeledValidated("Max. Amount (WART):  ", wart_validator))
    , inputBase(ui::LabeledValidated("", FundsValidator(a.precision)))
    , inputLimit(ui::LabeledValidated("Limit Price:  ", LimitValidator(a.precision, false)))
    , inputFee(ui::LabeledValidated("Fee (WART):  ", fee_validator))
    , toggle(Toggle(liquidity_actions, &side_selected))
    , btnCancel(Button("Cancel", [&]() { this->on_cancel(); }))
    , btnCreate(Button("Create", [&]() { this->on_create(); }))
{

    Add(Container::Vertical({ toggle, inputWart, inputLimit, inputFee,
        Container::Horizontal({ btnCancel, btnCreate }) }));
}
} // namespace ui

#include "api/endpoint.hpp"
#include "api/parse.hpp"
#include "communication/create_transaction.hpp"
#include "global.hpp"
#include "spdlog/spdlog.h"
#include "tui/gui.hpp"
#include "tui/tabs.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
namespace fs = std::filesystem;

std::vector<std::string> get_files(std::string path = ".")
{
    std::vector<std::string> out;
    for (const auto& entry : fs::directory_iterator(path)) {
        out.push_back(entry.path().filename().string());
    }
    return out;
}

[[nodiscard]] std::optional<std::string> read_file(const std::string filename)
{
    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp) {
        throw std::runtime_error(std::format("Cannot open file {}.", filename));
        return {};
    }

    std::string v;
    char buf[1024];
    while (size_t len = fread(buf, 1, sizeof(buf), fp))
        v.insert(v.end(), buf, buf + len);
    fclose(fp);
    return v;
}

Wallet load_wallet(const std::string filename = "tui_wallet.json")
{
    auto content { read_file(filename) };
    if (!content) {
        Wallet w;
        w.save(filename);
        return w;
    } else {
        try {
            return Wallet(*content);
        } catch (std::exception& e) {
            spdlog::error(e.what());
            auto errMsg {
                std::format("Cannot load wallet file '{}.'", filename)
            };
            throw std::runtime_error(errMsg);
        }
    }
}

void init_globals(ui::GUI& gui)
{
    global::init(gui, { .endpoint { "localhost", 3100 }, .wallet { load_wallet() } });
}

struct AccountBalance {
    struct {
        int id;
    } token;
};

int main()
{
    try {

        // using namespace global;
        // cout << wallet().get_wart_balance().total.to_string() << endl;
        // return 0;

        auto gui { ui::GUI::create_instance() };
        init_globals(*gui);

        // auto &r{global::globals().dataInterface.retrieval_context()};
        // auto& e { global::globals().dataInterface.retrieval_context().endpoint };
        // auto [pinHeight, pinHash] { e.get_pin() };
        // cout << "pinHeight: " << pinHeight.value() << " pinHash: " << serialize_hex(pinHash) << endl;

        // TransactionCreateBase(PinHeight pinHeight, NonceId nonceId, CompactUInt compactFee, Ts... ts, const Hash& pinHash, const PrivKey& pk, NonceReserved reserved = NonceReserved::zero())
        // try {
        //     cout << "Own address: " << global::wallet().address.to_string() << endl;
        //     Address toAddr("3661579d61abde5837a8686dc4d65348a2fc61b1fe5f4093");
        //     NonzeroWart wart { 1 };
        //     TransactionCreateContext ctx= global::wallet(). {
        //         .pinHash { pinHash },
        //         .pinHeight { pinHeight },
        //         .nonceId { 0 },
        //         .compactFee { CompactUInt::compact(Wart(0)) },
        //         .pk { global::wallet().privKey }
        //     };
        //     WartTransferCreate c1(ctx, toAddr, wart);
        //
        //     AssetHash ah { AssetHash::parse_throw("0e4825efffa294610d2ac376713e3bcc9b53d378e823834b64e5df01f75d3b0c") };
        //     Funds_uint64 amount(1);
        //     Funds_uint64 shares(1);
        //     TokenTransferCreate c2(ctx, ah, false, toAddr, amount);
        //     LimitSwapCreate c3(ctx, ah, true, amount, Price_uint64::from_double(1).value());
        //     LiquidityDepositCreate c4(ctx, ah, amount, wart);
        //     PinHeight cancelHeight(pinHeight);
        //     LiquidityWithdrawalCreate c5(ctx, ah, amount);
        //     NonceId cancelNonce(1);
        //     CancelationCreate c6(ctx, cancelHeight, cancelNonce);
        //     FundsDecimal assetSupply(Funds_uint64(1000000000), 3);
        //     AssetName assetName { "ASSET" };
        //     AssetCreationCreate c7(ctx, assetSupply, assetName);
        //     // DEFINE_CREATE_MESSAGE(AssetCreationCreate, ::block::labels::assetCreation, AssetSupplyEl, AssetNameEl)
        //     auto& c{c7};
        //     std::string json { c };
        //     cout << json << endl;
        //     cout << "Sent transaction: " << serialize_hex(e.send_transaction(c)) << endl;
        //     ;
        // } catch (std::runtime_error& e) {
        //     cout << "Error: " << e.what() << endl;
        // }
        // return 0;
        bool shutdown = false;
        std::condition_variable cv;
        std::mutex m;
        std::thread t([&]() {
            bool b { false };
            while (true) {
                {
                    std::unique_lock l(m);
                    if (cv.wait_for(l, std::chrono::seconds(1),
                            [&]() { return shutdown == true; }))
                        break;
                }
                b = !b;
                gui->set_connected(b);
                gui->set_unlocked(!b);
            }
            gui->terminate();
        });
        gui->run();
        {
            std::unique_lock l(m);
            shutdown = true;
        }
        cv.notify_one();
        t.join();
    } catch (const std::exception& e) {
        spdlog::error(e.what());
        abort();
    }
    return 0;
}

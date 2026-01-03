#pragma once
#include "wallet.hpp"
#include <memory>
namespace global {
struct Globals {
    std::unique_ptr<Wallet> walletptr;
    /* data */
};
inline Globals globals;
inline auto& wallet() { return *globals.walletptr; }
}

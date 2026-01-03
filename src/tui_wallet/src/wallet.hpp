#pragma once
#include "crypto/address.hpp"
#include "crypto/crypto.hpp"
#include "nlohmann/json_fwd.hpp"
#include <filesystem>

struct Wallet {
    PrivKey privKey;
    PubKey pubKey;
    Address address;
    std::string to_string() const;
    void save(const std::filesystem::path& path) const;

private:
    Wallet(nlohmann::json parsed);

public:
    Wallet(PrivKey k = PrivKey());
    Wallet(const std::string& jsonstr);
};

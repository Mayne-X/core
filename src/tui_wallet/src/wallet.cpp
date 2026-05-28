#include "wallet.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <spdlog/spdlog.h>

using nlohmann::json;

std::string Wallet::to_string() const
{
    json j;
    j["privateKey"] = privKey.to_string();
    j["publicKey"] = pubKey.to_string();
    j["address"] = address.to_string();
    return j.dump(1);
}
void Wallet::save(const std::filesystem::path& path) const
{
    if (std::filesystem::exists(path)) {
        throw std::runtime_error("Cannot create wallet, file '" + path.string() + "' already exists. You can specify a filename using the '-f' option.");
    }
    std::ofstream os(path);
    if (os.bad())
        throw std::runtime_error("Cannot create wallet file.");
    os << to_string();
    if (os.bad())
        throw std::runtime_error("Could not write wallet file");
}
Wallet::Wallet(nlohmann::json parsed)
    : Wallet([&] {
        return PrivKey(parsed["privateKey"].get<std::string>());
    }())
{
    std::string pubKeyString = parsed["publicKey"].get<std::string>();
    std::string addressString = parsed["address"].get<std::string>();
    if ((PubKey(pubKeyString) != pubKey) || (Address(addressString) != address)) {
        throw std::runtime_error("Inconsistent data.");
    }
}

Wallet::Wallet(PrivKey k)
    : privKey(k)
    , pubKey(privKey.pubkey())
    , address(pubKey.address())
{
}
Wallet::Wallet(const std::string& jsonstr)
    : Wallet(
          [&] {
              try {
                  return json::parse(jsonstr);
              } catch (std::exception& e) {
                  spdlog::error(e.what());
                  throw std::runtime_error("Cannot parse wallet JSON.");
              }
          }())
{
}

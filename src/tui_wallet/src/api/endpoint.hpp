#pragma once
#include "api/types/input.hpp"
#include "api/types/shared.hpp"
#include "crypto/hash.hpp"
#include "types.hpp"
#include <string>

class PinHeight;
class Endpoint {
    std::string host;
    uint16_t port;

public:
    Endpoint(std::string host, uint16_t port)
        : host(host)
        , port(port) { };
    api::FundsBalance get_balance(const std::string& account, api::TokenIdOrSpec token) const;
    api_types::TokenList token_complete(std::string_view namePrefix, std::string_view hashPrefix) const;
    api::FundsBalance wart_balance(const std::string& account) const;
    TxHash send_transaction(const std::string& txjson);
    std::pair<PinHeight, PinHash> get_pin();

private:
    [[nodiscard]] nlohmann::json extract_data(const std::string& json) const;
    [[nodiscard]] std::string http_get(const std::string& path) const;
    template<typename T>
   [[nodiscard]] T parse_get(const std::string& path) const;
    [[nodiscard]] nlohmann::json api_get(const std::string& path) const;
    [[nodiscard]] std::string http_post(const std::string& path, std::span<const uint8_t> postdata) const;
    [[nodiscard]] nlohmann::json api_post(const std::string& path, std::span<const uint8_t> postdata) const;
    [[nodiscard]] nlohmann::json api_post(const std::string& path, std::string_view s) const;
    std::runtime_error failed_msg() const;
};

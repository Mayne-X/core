#pragma once
#include "api/types/input.hpp"
#include "api/types/shared.hpp"
#include "crypto/hash.hpp"
#include <string>
#include <variant>
#include <vector>
class PinHeight;
class Endpoint {
    std::string host;
    uint16_t port;

public:
    struct Error {
        int code;
        std::string message;
    };
    Endpoint(std::string host, uint16_t port)
        : host(host)
        , port(port) { };
    api::FundsBalance get_balance(const std::string& account, api::TokenIdOrSpec token) const;
    api::FundsBalance get_wart_balance(const std::string& account) const;
    std::variant<TxHash, Error> send_transaction(const std::string& txjson);
    std::pair<PinHeight, PinHash> get_pin();

private:
    bool http_get(const std::string& get, std::string& out) const;
    int http_post(const std::string& path, const std::vector<uint8_t>& postdata, std::string& out);
    std::runtime_error failed_msg() const;
};

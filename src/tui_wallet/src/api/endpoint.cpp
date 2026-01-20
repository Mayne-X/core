#include "endpoint.hpp"
#include "block/chain/height.hpp"
#include "general/hex.hpp"
#include "httplib.hpp"
#include "log.hpp"
#include "nlohmann/json.hpp"
#include <algorithm>
#include <iostream>
using namespace std;
using namespace nlohmann;

std::string Endpoint::http_get(const std::string& path) const
{
    auto ref { log_request("GET " + host + ":" + std::to_string(port) + path) };
    httplib::Client cli(host, port);
    cli.set_read_timeout(10);
    auto res = cli.Get(path);
    { // set success
        auto p { ref.lock() };
        if (p) {
            p->set_success(res);
        }
    }
    if (res) {
        return { std::move(res->body) };
    }
    throw std::runtime_error("GET request failed");
}

nlohmann::json Endpoint::extract_data(const std::string& json) const
{
    try {
        std::string error;
        auto parsed = json::parse(json);
        auto iter = parsed.find("error");
        if (iter != parsed.end() && !iter->is_null()) {
            error = iter->get<string>();
        }
        auto code { parsed["code"].get<int32_t>() };
        if (code != 0 || !error.empty()) {
            if (error.empty()) {
                throw std::runtime_error(std::format("Error without details returned by API, code {}", code));
            }
            throw std::runtime_error(std::format("API error \"{}\", code {}", error, code));
        } else {
            auto j = parsed["data"];
            bool b { j.is_object() };
            auto s { j.size() };
            return j;
        }
    } catch (...) {
        throw std::runtime_error("API response is malformed.");
    }
}

json Endpoint::api_get(const std::string& path) const
{
    return extract_data(http_get(path));
}

json Endpoint::api_post(const std::string& path, std::span<const uint8_t> postdata) const
{
    return extract_data(http_post(path, postdata));
}

json Endpoint::api_post(const std::string& path, std::string_view s) const
{
    std::span<const uint8_t> sp(reinterpret_cast<const uint8_t*>(s.data()), s.size());
    return api_post(path, sp);
}

std::string Endpoint::http_post(const std::string& path, std::span<const uint8_t> postdata) const
{
    auto ref { log_request("POST " + path) };
    httplib::Client cli(host, port);
    cli.set_read_timeout(10);
    auto res = cli.Post(path, (const char*)postdata.data(), postdata.size(), ""s);
    { // set success
        auto p { ref.lock() };
        if (p) {
            p->set_success(res);
        }
    }
    if (res) {
        return { std::move(res->body) };
    }
    throw std::runtime_error("POST request failed");
}

std::pair<PinHeight, PinHash> Endpoint::get_pin()
{
    auto data(api_get("/chain/head"));
    std::string h = data["pinHash"].get<std::string>();
    auto pinHash { Hash::parse_throw(h) };

    auto v { data["pinHeight"].get<int32_t>() };
    if (auto pinHeight = Height(v).pin_height())
        return make_pair(*pinHeight, PinHash(pinHash));
    throw std::runtime_error(std::format("Invalid pinHeight {}.", v));
}

api::FundsBalance Endpoint::get_balance(const std::string& account, api::TokenIdOrSpec token) const
{
    std::string url = "/account/" + account + "/balance/" + token.to_string();
    auto data(api_get("/chain/head"));
    auto l { [](json& balance) {
        TokenPrecision p(balance["precision"].get<uint8_t>());
        Funds_uint64 f(balance["u64"].get<uint64_t>());
        return FundsDecimal { f, p };
    } };
    auto iter = data.find("balance");
    auto bal = data["balance"];
    return { .total { l(bal["total"]) }, .locked { l(bal["locked"]) } };
}

api::FundsBalance Endpoint::get_wart_balance(const std::string& account) const
{
    return get_balance(account, TokenId::WART);
}

api_types::TokenList Endpoint::token_list(const std::string& prefix)
{
    api_types::TokenList res(prefix);
    if (!std::all_of(prefix.begin(), prefix.end(), [](char c) {
            return isalnum(c);
        }))
        return res; // return empty list
    std::string url { "/token/complete/" + prefix };
    auto data(api_get(url));
    for (auto& m : data["matches"]) {
        res.entries.push_back({
            .hash { m["hash"].get<std::string>() },
            .height = m["height"].get<uint32_t>(),
            .name { m["name"].get<std::string>() },
        });
    }
    return res;
}
auto Endpoint::send_transaction(const std::string& txjson) -> TxHash
{
    cout << "=====DEBUG INFO TRANSACTION JSON=====\n"
         << txjson << "\n"
         << "=====================================" << endl;

    std::string url = "/transaction/add";
    auto hex_str(api_post(url, txjson)
            .at("txHash")
            .get<std::string>());
    return TxHash { Hash { hex_to_arr<32>(hex_str) } };
}

std::runtime_error Endpoint::failed_msg() const
{
    return std::runtime_error { "API request to host " + host + " at port " + std::to_string(port) + " failed. Are you running the node with RPC endpoint enabled?" };
};

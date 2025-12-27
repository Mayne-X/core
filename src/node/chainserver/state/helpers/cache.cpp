#include "cache.hpp"
#include "chainserver/db/chain_db.hpp"
#include "crypto/address.hpp"

namespace chainserver {

template <typename Key, typename Value>
const Value* MapCached<Key, Value>::lookup(const ChainDB& db, const Key& key)
{
    auto iter = map.lower_bound(key);
    if (iter != map.end() && iter->first == key)
        return &iter->second;
    if (auto a { db.lookup_asset(key) })
        return &map.emplace_hint(iter, key, *a)->second;
    return nullptr;
}

template <typename Key, typename Value>
const Value& MapCached<Key, Value>::fetch_throw(const ChainDB& db, const Key& key)
{
    return map.try_emplace_lambda(key, [&] { return db.fetch_throw<Value>(key); })
        .first->second;
}

template <typename Key, typename Value>
const Value& MapCached<Key, Value>::fetch_existing(const ChainDB& db, const Key& key)
{
    return map.try_emplace_lambda(key, [&] { return db.fetch_existing<Value>(key); })
        .first->second;
}

const AssetDetail& AssetCacheById::fetch_existing(AssetId id)
{
    auto iter = map.find(id);
    if (iter != map.end())
        return iter->second;
    return map.emplace(id, db.fetch_existing<AssetDetail>(id)).first->second;
}
const AssetDetail& AssetCacheByHash::fetch(AssetHash h)
{
    auto iter = map.find(h);
    if (iter != map.end())
        return iter->second;
    return map.emplace(h, db.fetch_existing<AssetDetail>(h)).first->second;
}
const AssetDetail* AssetCacheByHash::lookup(AssetHash h)
{
    auto iter = map.find(h);
    if (iter != map.end())
        return &iter->second;
    if (auto a { db.lookup_asset(h) })
        return &map.emplace(h, *a).first->second;
    return nullptr;
}

const wrt::optional<Address>& AddressCache::get(AccountId id)
{
    auto iter { map.find(id) };
    if (iter == map.end())
        iter = map.emplace(id, db.lookup_address(id)).first;
    return iter->second;
}

const Address& AddressCache::fetch(AccountId id)
{
    if (auto& o { get(id) })
        return o.value();
    throw std::runtime_error("Cannot fetch address with id" + std::to_string(id.value()) + ".");
}

Funds_uint64 BalanceCache::operator[](AccountToken at)
{
    auto iter { map.find(at) };
    if (iter == map.end())
        iter = map.emplace(at, db.get_free_balance(at)).first;
    return iter->second;
}

const AssetDetail& DBCache::existing_asset(AssetId id)
{
    return caches.get<AssetsById>().fetch_existing(db, id);
}
const Address& DBCache::existing_address(AccountId id)
{
    return caches.get<AddressById>().fetch_existing(db, id);
}

const AssetDetail* DBCache::lookup_asset(const AssetHash& hash)
{
    return caches.get<AssetsByHash>().lookup(db, hash);
}
const AssetDetail& DBCache::fetch_asset_throw(const AssetHash& hash)
{
    return caches.get<AssetsByHash>().fetch_throw(db, hash);
}
}

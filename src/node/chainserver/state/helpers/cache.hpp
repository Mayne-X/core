#pragma once
#include "block/body/account_id.hpp"
#include "block/chain/history/history.hpp"
#include "block/chain/history/index.hpp"
#include "chainserver/db/types_fwd.hpp"
#include "defi/token/account_token.hpp"
#include "defi/token/info.hpp"
#include "wrt/map.hpp"

namespace chainserver {
template <typename Key, typename Value>
class MapCached {
public:
    void clear() { map.clear(); }
    [[nodiscard]] const Value* lookup(const ChainDB& db, const Key& key);
    [[nodiscard]] const Value& fetch_throw(const ChainDB& db, const Key& key);
    [[nodiscard]] const Value& fetch_existing(const ChainDB& db, const Key& key);

protected:
    mutable wrt::map<Key, Value> map;
};

template <typename Key, typename Value>
class DBCacheBase {
public:
    DBCacheBase(const ChainDB& db)
        : db(db)
    {
    }
    void clear() { map.clear(); }

protected:
    const ChainDB& db;
    mutable std::map<Key, Value> map;
};

class AddressCache : public DBCacheBase<AccountId, std::optional<Address>> {
public:
    using DBCacheBase::DBCacheBase;

    const std::optional<Address>& get(AccountId);
    const Address& fetch(AccountId);
};

class BalanceCache : public DBCacheBase<AccountToken, Funds_uint64> {
public:
    using DBCacheBase::DBCacheBase;
    [[nodiscard]] Funds_uint64 operator[](AccountToken at);
};

class AssetCacheById : public DBCacheBase<AssetId, AssetDetail> {
public:
    using DBCacheBase::DBCacheBase;
    [[nodiscard]] const AssetDetail& fetch_existing(AssetId id);
};

class AssetCacheByHash : public DBCacheBase<AssetHash, AssetDetail> {
public:
    using DBCacheBase::DBCacheBase;
    [[nodiscard]] const AssetDetail& fetch(AssetHash);
    [[nodiscard]] const AssetDetail* lookup(AssetHash);
};

class DBCache {
    template <typename... Ts>
    struct CacheCollection : public Ts... {
        template <typename T>
        requires(std::is_same_v<T, Ts> || ...)
        auto& get() const
        {
            return *static_cast<const T*>(this);
        }
        template <typename T>
        requires(std::is_same_v<T, Ts> || ...)
        auto& get()
        {
            return *static_cast<T*>(this);
        }
        void clear()
        {
            (get<Ts>().clear(), ...);
        }
    };
    using AssetsById = MapCached<AssetId, AssetDetail>;
    using AssetsByHash = MapCached<AssetHash, AssetDetail>;
    using AddressById = MapCached<AccountId, Address>;
    using Caches = CacheCollection<
        AssetsById,
        AddressById,
        AssetsByHash>;

public:
    DBCache(const ChainDB& db)
        : db(db)
        , balance(db)
        , assetsByHash(db)
    {
    }
    void clear()
    {
        caches.clear();
        balance.clear();
        assetsByHash.clear();
    }
    [[nodiscard]] const AssetDetail& existing_asset(AssetId id);
    [[nodiscard]] const Address& existing_address(AccountId id);
    [[nodiscard]] const AssetDetail* lookup_asset(const AssetHash& hash);
    [[nodiscard]] const AssetDetail& fetch_asset_throw(const AssetHash& hash);

    const ChainDB& db;
    Caches caches;
    BalanceCache balance;

private:
    AssetCacheByHash assetsByHash;
};

}

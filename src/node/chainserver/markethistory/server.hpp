#pragma once
// #include "../api_types.hpp"
// #include "api/callbacks.hpp"
// #include "api/events/subscription_fwd.hpp"
// #include "api/types/height_or_hash.hpp"
#include "api/enable_api.hpp"
#include "api/types/input.hpp"
#include "trades_db.hpp"
#include "wrt/variant.hpp"
#include <condition_variable>
#include <list>
#include <shared_mutex>
#include <thread>
#include <vector>

namespace market_history {
#define LIST_API_TYPES(XX) \
    XX(MiningAppend, void)

namespace chainserver {
DEFINE_TYPE_COLLECTION(APITypes, LIST_API_TYPES);
}
#undef LIST_API_TYPES

class ReaderThreadpool;

using CandlesCallback = std::function<void(Result<std::vector<Candle>>)>;
using TradesCallback = std::function<void(Result<std::vector<Candle>>)>;
struct GetCandles {
    api::AssetIdOrHash assetHash;
    std::string interval;
    std::optional<Timestamp> first;
    std::optional<Timestamp> last;
    std::optional<size_t> N;
    CandlesCallback callback;
};

struct HasAsset {
    api::AssetIdOrHash asset;
};
struct GetTradesBase : HasAsset {
    TradesCallback callback;
    GetTradesBase(api::AssetIdOrHash asset, CandlesCallback callback)
        : HasAsset(std::move(asset))
        , callback(std::move(callback))
    {
    }
};

struct GetTradesFrom : GetTradesBase {
    NonzeroHeight first;
    size_t N;
};
struct GetTradesTo : GetTradesBase {
    NonzeroHeight last;
    size_t N;
};
struct GetTradesRange : GetTradesBase {
    NonzeroHeight first;
    NonzeroHeight last;
};
struct GetTradesLatest : GetTradesBase {
    size_t N;
};

struct GetCandlesBase : HasAsset {
    CandlesCallback callback;
    Interval interval;
    GetCandlesBase(api::AssetIdOrHash asset, CandlesCallback callback, Interval interval)
        : HasAsset(std::move(asset))
        , callback(std::move(callback))
        , interval(std::move(interval))
    {
    }
};
struct GetCandlesFrom : public GetCandlesBase {
    Timestamp first;
    size_t N;
};
struct GetCandlesTo : public GetCandlesBase {
    Timestamp last;
    size_t N;
};
struct GetCandlesRange : public GetCandlesBase {
    Timestamp first;
    Timestamp last;
};
struct GetCandlesLatest : public GetCandlesBase {
    size_t N;
};

using ReaderEvent = wrt::variant<
    GetTradesRange,
    GetTradesFrom,
    GetTradesTo,
    GetTradesLatest,
    GetCandlesRange,
    GetCandlesFrom,
    GetCandlesTo,
    GetCandlesLatest>;
class Reader {
public:
    Reader(MarketReaderDB&& db, ReaderThreadpool& parent)
        : db(std::move(db))
        , parent(parent)
    {
        t = std::jthread([&] { work(); });
    }
    ~Reader()
    {
        shutdown();
    }
    void shutdown()
    {
    }

private:
    void work();
    void dispatch(ReaderEvent&& event);
    Result<Asset> normalize(const api::AssetIdOrHash& asset);
    void handle_event(const Asset&, GetCandlesRange&&);
    void handle_event(const Asset&, GetCandlesFrom&&);
    void handle_event(const Asset&, GetCandlesTo&&);
    void handle_event(const Asset&, GetCandlesLatest&&);
    void handle_event(const Asset&, GetTradesRange&&);
    void handle_event(const Asset&, GetTradesFrom&&);
    void handle_event(const Asset&, GetTradesTo&&);
    void handle_event(const Asset&, GetTradesLatest&&);

private:
    MarketReaderDB db;
    ReaderThreadpool& parent;
    std::jthread t;
};

class ReaderThreadpool {
    friend class Reader;

public:
    ReaderThreadpool(MarketDb& db, size_t N);
    ReaderThreadpool(const ReaderThreadpool&) = delete; // no copy, address needs to stay constant as we have references in the Readers.
    void defer(ReaderEvent&& e)
    {
        std::lock_guard l(m);
        readerEvents.push_back(std::move(e));
        cv.notify_one();
    }
    void shutdown()
    {
        std::lock_guard l(m);
        _shutdown = true;
        cv.notify_all();
    }
    MarketReaderDB clone_reader() const;

private:
    bool _shutdown { false };
    std::vector<std::unique_ptr<Reader>> readers;
    std::condition_variable cv;
    std::mutex m;
    std::list<ReaderEvent> readerEvents;
};

class MarketHistoryServer {
    friend class ReaderThreadpool;

private:
    // using variant_t = wrt::variant<
public:
    using Event = wrt::variant<int>;

private:
    MarketHistoryServer(MarketDb& db);
    void defer(Event e)
    {
        std::lock_guard l(m);
        events.push_back(std::move(e));
        cv.notify_all();
    }
    void defer(ReaderEvent e)
    {
        readers.defer(std::move(e));
    }
    void handle_event(int) { };

private:
    MarketDb& db;
    std::condition_variable cv;
    std::mutex m;
    std::shared_mutex sqliteLock;
    std::vector<Event> events;

    ReaderThreadpool readers;
};

}
using MarketHistoryServer = market_history::MarketHistoryServer;

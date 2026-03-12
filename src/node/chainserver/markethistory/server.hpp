#pragma once
#include "api/enable_api.hpp"
#include "api/types/input.hpp"
#include "api/types/opt_param.hpp"
#include "trades_db.hpp"
#include "wrt/variant.hpp"
#include <condition_variable>
#include <list>
#include <shared_mutex>
#include <thread>
#include <vector>

using CandlesCallback = std::function<void(Result<api::CandlesVector>)>;
using TradesCallback = std::function<void(Result<api::TradesVector>)>;
#define LIST_API_TYPES(XX)                                        \
    XX(GetCandles, api::CandlesVector, api::AssetIdOrHash, asset, \
        std::string, interval, OptParam<Timestamp>, from,         \
        OptParam<Timestamp>, to, OptParam<size_t>, N)             \
    XX(GetTrades, api::TradesVector, api::AssetIdOrHash, asset,   \
        OptParam<NonzeroHeight>, from, OptParam<NonzeroHeight>,   \
        to, OptParam<size_t>, N)

DEFINE_TYPE_COLLECTION(APIReadTypes, LIST_API_TYPES);
#undef LIST_API_TYPES
namespace market_history {

class ReaderThreadpool;

struct HasAsset {
    api::AssetIdOrHash asset;
};
struct GetTradesBase : HasAsset {
    TradesCallback callback;
    GetTradesBase(api::AssetIdOrHash asset, TradesCallback callback)
        : HasAsset(std::move(asset))
        , callback(std::move(callback))
    {
    }
};

struct GetTradesFrom : GetTradesBase {
    NonzeroHeight from;
    size_t N;
};
struct GetTradesTo : GetTradesBase {
    NonzeroHeight to;
    size_t N;
};
struct GetTradesRange : GetTradesBase {
    NonzeroHeight from;
    NonzeroHeight to;
};
struct GetTradesLatest : GetTradesBase {
    size_t N;
};

struct GetCandlesBase : HasAsset {
    Interval interval;
    CandlesCallback callback;
    GetCandlesBase(api::AssetIdOrHash asset, Interval interval, CandlesCallback callback)
        : HasAsset(std::move(asset))
        , interval(std::move(interval))
        , callback(std::move(callback))
    {
    }
};
struct GetCandlesFrom : public GetCandlesBase {
    Timestamp from;
    size_t N;
};
struct GetCandlesTo : public GetCandlesBase {
    Timestamp to;
    size_t N;
};
struct GetCandlesRange : public GetCandlesBase {
    Timestamp from;
    Timestamp to;
};
struct GetCandlesLatest : public GetCandlesBase {
    size_t N;
};

using ReaderEventInternal = wrt::variant<
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
    void dispatch(ReaderEventInternal&& event);
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

using ReaderEvent = wrt::variant<GetCandles, GetTrades>;

class ReaderThreadpool : public enable_api_methods<ReaderThreadpool, APIReadTypes> {
    friend class Reader;

public:
    ReaderThreadpool(MarketDb& db, size_t N);
    ReaderThreadpool(const ReaderThreadpool&) = delete; // no copy, address needs to stay constant as we have references in the Readers.
    void defer(ReaderEventInternal&& e)
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
    std::list<ReaderEventInternal> readerEvents;
};

class MarketHistoryServer {
    friend class ReaderThreadpool;

public:
    using Event = wrt::variant<int>;

    void api_call(GetCandles::Object&& e);
    void api_call(GetTrades::Object&& e);
    void defer(Event e)
    {
        std::lock_guard l(m);
        events.push_back(std::move(e));
        cv.notify_all();
    }
    template <typename T>

    static constexpr bool supports = ReaderThreadpool::supports<T>;

    MarketHistoryServer(MarketDb& db);

private:
    ReaderEventInternal wrap_event_throw(GetCandles::Object&& e);
    ReaderEventInternal wrap_event_throw(GetTrades::Object&& e);
    void defer(ReaderEventInternal e)
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

#include "server.hpp"

namespace market_history {

void Reader::work()
{
    std::unique_lock l(parent.m);
    while (true) {
        parent.cv.wait(l, [&] {
            return parent._shutdown || !parent.readerEvents.empty();
        });
        // take one event at a time
        auto event { parent.readerEvents.front() };
        parent.readerEvents.pop_front();
        l.unlock();
        dispatch(std::move(event));
    }
}

Result<Asset> Reader::normalize(const api::AssetIdOrHash& asset)
{
    return asset.visit_overload(
        [&](const AssetId& id) -> Result<Asset> {
            auto a { db.get_asset(id) };
            if (a.has_value()) {
                return *a;
            }
            return Error(EASSETIDNOTFOUND);
        },
        [&](const AssetHash& hash) -> Result<Asset> {
            auto a { db.get_asset(hash) };
            if (a.has_value()) {
                return *a;
            }
            return Error(EASSETHASHNOTFOUND);
        });
}

void Reader::dispatch(ReaderEventInternal&& event)
{
    std::move(event).visit([&](auto&& e) {
        // open transaction to have consistent view on database
        // across multiple queries while the writer can change
        // the database (we use WAL SQLite feature)
        auto tx { db.transaction() };
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::derived_from<T, HasAsset>) {
            auto a { normalize(e.asset) };
            if (!a)
                return e.callback(a.error());
            if constexpr (std::derived_from<T, GetCandlesBase>) {
                return handle_event(*a, std::move(e));
            }
            handle_event(*a, std::move(e));
        } else {
            handle_event(std::move(e));
        }
        tx.commit(); // close transaction (does not write anything)
    });
}

void Reader::handle_event(const Asset& a, GetTradesRange&& e)
{
    e.callback(db.get_trades_range(a.id, e.from, e.to));
}
void Reader::handle_event(const Asset& a, GetTradesFrom&& e)
{
    e.callback(db.get_trades_from(a.id, e.from, e.N));
}
void Reader::handle_event(const Asset& a, GetTradesTo&& e)
{
    e.callback(db.get_trades_to(a.id, e.to, e.N));
}
void Reader::handle_event(const Asset& a, GetTradesLatest&& e)
{
    e.callback(db.get_trades_latest(a.id, e.N));
}
void Reader::handle_event(const Asset& a, GetCandlesRange&& e)
{
    e.callback(db.get_candles_range(a.id, e.interval, e.from, e.to));
}
void Reader::handle_event(const Asset& a, GetCandlesFrom&& e)
{
    e.callback(db.get_candles_from(a.id, e.interval, e.from, e.N));
}
void Reader::handle_event(const Asset& a, GetCandlesTo&& e)
{
    e.callback(db.get_candles_to(a.id, e.interval, e.to, e.N));
}
void Reader::handle_event(const Asset& a, GetCandlesLatest&& e)
{
    e.callback(db.get_candles_latest(a.id, e.interval, e.N));
}

ReaderThreadpool::ReaderThreadpool(MarketDb& db, size_t N)
{
    for (size_t i { 0 }; i < N; ++i)
        readers.push_back(std::make_unique<Reader>(db.clone_reader(), *this));
}
namespace {
constexpr size_t MAX_N = 200;
constexpr size_t DEFAULT_N = 100;
}

ReaderEventInternal MarketHistoryServer::wrap_event_throw(GetTrades::Object&& o)
{
    auto& req { o.request };
    if (req.N() > MAX_N)
        throw Error(ERANGETOOBIG);

    auto base { [&] { return GetTradesBase { req.asset(), std::move(o.callback) }; } };
    if (req.to()) {
        if (req.from()) {
            if (req.N()) // cannot have all 3 args
                throw Error(EINVARGCOMB);

            if (*req.to() < *req.from())
                throw Error(EINVRANGE);
            size_t N = req.to()->value() - req.from()->value();
            if (N > MAX_N)
                throw Error(ERANGETOOBIG);
            return GetTradesRange { base(), *req.from(), *req.to() };
        }
        return GetTradesTo { base(), *req.to(), req.N().value_or(DEFAULT_N) };
    }
    if (req.from()) {
        return GetTradesFrom { base(), *req.from(), req.N().value_or(DEFAULT_N) };
    }
    return GetTradesLatest { base(), req.N().value_or(DEFAULT_N) };
}

ReaderEventInternal MarketHistoryServer::wrap_event_throw(GetCandles::Object&& o)
{
    auto& req { o.request };
    auto i { market_history::Interval::from_slug(req.interval()) };
    if (!i) // could not parse interval
        throw Error(EINVINTERVAL);
    auto base { [&] { return GetCandlesBase { req.asset(), *i, std::move(o.callback) }; } };
    if (req.N() > MAX_N)
        throw Error(ERANGETOOBIG);

    if (req.to()) {
        if (req.from()) {
            if (req.N()) // cannot have all 3 args
                throw Error(EINVARGCOMB);

            if (*req.to() < *req.from())
                throw Error(EINVRANGE);
            size_t N = (req.to()->value() - req.from()->value()) / i->seconds();
            if (N > MAX_N)
                throw Error(ERANGETOOBIG);
            return GetCandlesRange {
                base(), *req.from(), *req.to()
            };
        }
        return GetCandlesTo {
            base(), *req.to(), req.N().value_or(DEFAULT_N)
        };
    }
    if (req.from()) {
        return GetCandlesFrom {
            base(), *req.from(), req.N().value_or(DEFAULT_N)
        };
    }
    return GetCandlesLatest { base(), req.N().value_or(DEFAULT_N) };
}

void MarketHistoryServer::api_call(GetCandles::Object&& ev)
{
    try {
        defer(wrap_event_throw(std::move(ev)));
    } catch (Error e) {
        ev.callback(e);
    }
}
void MarketHistoryServer::api_call(GetTrades::Object&& ev)
{
    try {
        defer(wrap_event_throw(std::move(ev)));
    } catch (Error e) {
        ev.callback(e);
    }
}

MarketHistoryServer::MarketHistoryServer(MarketDb& db)
    : db(db)
    , readers(db, 1)
{
}
}

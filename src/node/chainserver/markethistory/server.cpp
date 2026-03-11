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

void Reader::dispatch(ReaderEvent&& event)
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
                // auto i { market_history::Interval::from_slug(e.interval) };
                // if (!i) // could not parse interval
                //     return e.callback(Error(EINVINTERVAL));
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
    e.callback(db.get_trades_range(a.id, e.first, e.last));
}
void Reader::handle_event(const Asset& a, GetTradesFrom&& e)
{
    e.callback(db.get_trades_from(a.id, e.first, e.N));
}
void Reader::handle_event(const Asset& a, GetTradesTo&& e)
{
    e.callback(db.get_trades_to(a.id, e.last, e.N));
}
void Reader::handle_event(const Asset& a, GetTradesLatest&& e)
{
    e.callback(db.get_trades_latest(a.id, e.N));
}
void Reader::handle_event(const Asset& a, GetCandlesRange&& e)
{
    e.callback(db.get_candles_range(a.id, e.interval, e.first, e.last));
}
void Reader::handle_event(const Asset& a, GetCandlesFrom&& e)
{
    e.callback(db.get_candles_from(a.id, e.interval, e.first, e.N));
}
void Reader::handle_event(const Asset& a, GetCandlesTo&& e)
{
    e.callback(db.get_candles_to(a.id, e.interval, e.last, e.N));
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

MarketHistoryServer::MarketHistoryServer(MarketDb& db)
    : db(db)
    , readers(1)
{
}
}

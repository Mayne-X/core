#include "trades_db.hpp"
#include "db/sqlite.hpp"
#include "spdlog/spdlog.h"
namespace market_history {

namespace {
SQLite::Database create_database(const std::string& path)
{
    spdlog::debug("Opening market database \"{}\"", path);
    SQLite::Database out(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    out.exec(
        "PRAGMA foreign_keys = ON;"
        "PRAGMA journal_mode = WAL;"
        "CREATE TABLE IF NOT EXISTS `Assets` (`id` INTEGER NOT NULL, `hash` INTEGER UNIQUE, `latestHeight` INTEGER NOT NULL, PRIMARY KEY(`id`));"
        "CREATE TABLE IF NOT EXISTS `Blocks` (`height` INTEGER, `hash` INTEGER UNIQUE, PRIMARY KEY(`height`));"
        "CREATE INDEX `latestHeightIndex` ON `Assets` ( `latestHeight`);");
    return out;
}

[[nodiscard]] inline std::string candles_table(AssetId assetId, Interval interval)
{
    return std::format("Candles_{}_{}", assetId.value(), interval.slug());
}

[[nodiscard]] inline std::string trades_table(AssetId assetId)
{
    return std::format("Trades_{}", assetId.value());
}
void candle_add_trade(Candle& c, const TradeAmount& ta)
{
    auto price { ta.price() };
    c.close = price;
    if (c.high < price)
        c.high = price;
    if (c.low > price)
        c.low = price;
    c.base += ta.base();
    c.quote += ta.quote();
}
}

MarketReaderDB::MarketReaderDB(SQLite::Database&& db)
    : db(std::move(db))
    , stmtSelectAssetById(db, "SELET id, hash, latestHeight FROM Assets WHERE id = ?")
    , stmtSelectAssetByHash(db, "SELET id, hash, latestHeight FROM Assets WHERE hash = ?")
{
}

wrt::optional<Asset> MarketReaderDB::get_asset(AssetId id) const
{
    return stmtSelectAssetById.one(id.value()).process([](auto& row) {
        return Asset { .id = row[0], .hash = row[1], .latestHeight = row[2] };
    });
}
wrt::optional<Asset> MarketReaderDB::get_asset(AssetHash hash) const
{
    return stmtSelectAssetByHash.one(hash).process(
        [](auto& row) { return Asset {
                            .id { row[0] },
                            .hash { row[1] },
                            .latestHeight { row[2] }
                        }; });
}
std::vector<Candle> MarketReaderDB::get_trades_range(AssetId, NonzeroHeight from, NonzeroHeight to) const{

}
std::vector<Candle> MarketReaderDB::get_trades_from(AssetId, NonzeroHeight from, size_t n) const{

}
std::vector<Candle> MarketReaderDB::get_trades_to(AssetId, NonzeroHeight to, size_t n) const{

}
std::vector<Candle> MarketReaderDB::get_trades_latest(AssetId, size_t n) const{

}

std::vector<Candle> MarketReaderDB::get_candles_range(AssetId, Interval interval, Timestamp from, Timestamp to) const{

}
std::vector<Candle> MarketReaderDB::get_candles_from(AssetId, Interval interval, Timestamp from, size_t n) const{

}
std::vector<Candle> MarketReaderDB::get_candles_to(AssetId, Interval interval, Timestamp to, size_t n) const{

}
std::vector<Candle> MarketReaderDB::get_candles_latest(AssetId, Interval interval, size_t n) const{

}
std::vector<Candle> MarketReaderDB::get_candles_range(AssetId assetId, Interval interval, uint64_t beginTime, uint64_t endTime) const
{
    auto tableName { candles_table(assetId, interval) };
    std::string query { std::format("SELECT timestamp, height, open, high, low, close, base, quote FROM {} WHERE timestamp >= ? and timestamp < ? ORDER BY timestamp ASC", tableName) };
    Statement stmt(db, query);
    return stmt.all([](sqlite::Row&& row) {
        return Candle {
            .timestamp { row[0] },
            .height { row[1] },
            .open { row[2] },
            .high { row[3] },
            .low { row[4] },
            .close { row[5] },
            .base { row[6] },
            .quote { row[7] },
        };
    },
        beginTime, endTime);
}

std::vector<Candle> MarketReaderDB::get_candles_latest(AssetId assetId, Interval interval, size_t n) const
{
    auto tableName { candles_table(assetId, interval) };
    std::string query { std::format("SELECT timestamp, height, open, high, low, close, base, quote FROM {} WHERE ORDER BY timestamp DESC LIMIT ?", tableName) };
    Statement stmt(db, query);
    return stmt.all([](sqlite::Row&& row) {
        return Candle {
            .timestamp { row[0] },
            .height { row[1] },
            .open { row[2] },
            .high { row[3] },
            .low { row[4] },
            .close { row[5] },
            .base { row[6] },
            .quote { row[7] },
        };
    },
        n);
}

MarketReaderDB MarketReaderDB::clone_reader() const
{
    return { SQLite::Database(db.getFilename(), SQLite::OPEN_READONLY) };
}

void MarketDB::aggregate_into_candles(AssetId assetId, Interval interval, const Trade tr, Timestamp ts)
{
    auto c { get_latest_candle(assetId, interval) };
    auto begin { ts.floor(interval.seconds()) };
    if (c && c->timestamp >= begin) { // update old candle
        candle_add_trade(*c, tr);
        auto table { candles_table(assetId, interval) };
        Statement stmt(db, std::format("UPDATE {} SET high=?, low=?, close=?, base=?, quote=? WHERE timestamp=?", table));
        stmt.run(c->high, c->low, c->close, c->base, c->quote, c->timestamp);
    } else {
        double price { tr.price() };
        Candle c {
            .timestamp = begin,
            .height { tr.height },
            .open = price,
            .high = price,
            .low = price,
            .close = price,
            .base = tr.base(),
            .quote = tr.quote(),
        };
        insert_candle(assetId, interval, c);
    }
}
// template <Interval interval>
// void MarketDB::remove_from_latest_candle(AssetId assetId, const Trade t)
// {
//     auto c { get_latest_candle(assetId, interval) };
//     assert(c);
//     assert(c->base >= t.base());
//     assert(c->quote >= t.quote());
//     c->base -= t.base();
//     c->quote -= t.quote();
//     assert((c->base == 0) == (c->quote == 0));
//     if (c->base == 0) { // remove whole candle
//     }
// }

void MarketDB::insert_trade(const Asset& asset, const Trade& tr, Timestamp ts)
{
    if (asset.latestHeight.is_zero())
        create_tables(asset.id);
    aggregate_into_candles(asset.id, FIVEMIN(), tr, ts);
    aggregate_into_candles(asset.id, ONEHOUR(), tr, ts);
    aggregate_into_candles(asset.id, ONEDAY(), tr, ts);
    stmtSetLatestHeight.run(tr.height, asset.id);
}
void MarketDB::rollback(AssetId nextAssetId, NonzeroHeight nextHeight, Timestamp nextTimestamp)
{
    // assets which were inserted starting from nextAssetId can be deleted directly
    delete_assets_from(nextAssetId, nextHeight);

    // for the remaining assets, roll back those that have latestHeight>= nextHeight
    stmtSelectAssetIdsByLatestHeight.for_each(
        [&](auto& row) {
            AssetId id(row[0]);
            asset_rollback(id, nextHeight, nextTimestamp);
        },
        nextHeight);
}

void MarketDB::erase_interval_candles_from_height(AssetId assetId, Interval interval, Timestamp from)
{
    auto table { candles_table(assetId, interval) };
    sqlite::Statement stmt(db, std::format("DELETE FROM {} WHERE timestamp >= ?", table));
    stmt.run(from);
}

auto MarketDB::asset_erase_candles_from(AssetId assetId, NonzeroHeight blockHeight, Timestamp from) -> CandleBegin
{
    std::optional<CandleBegin> cb;
    const auto tableName { candles_table(assetId, FIVEMIN()) };
    {
        sqlite::Statement stmt(db, std::format("SELECT timestamp, height FROM {} WHERE timestamp>=? ORDER BY timestamp ASC", tableName));
        stmt.for_each_continue([&](auto&& row) {
            auto intervalHeight { Height(row[1]).nonzero() };
            assert(intervalHeight.has_value());
            if (*intervalHeight <= blockHeight) {
                cb = CandleBegin { .timestamp = row[0], .height = row[1] };
                return true; // continue;
            }
            return false; // stop the for_each_continue loop.
        },
            from.floor(FIVEMIN::seconds));
    } // local scope end
    assert(cb.has_value()); // there must be values in the candle containing the trade to which the passed args corresponsd.
    //
    auto deleteFromTimestamp { cb->timestamp };
    sqlite::Statement stmt(db, std::format("DELETE FROM {} WHERE timestamp >= ?)", tableName));
    stmt.run(deleteFromTimestamp);
    erase_interval_candles_from_height(assetId, FIVEMIN(), deleteFromTimestamp);
    erase_interval_candles_from_height(assetId, ONEHOUR(), deleteFromTimestamp.floor(ONEHOUR::seconds));
    erase_interval_candles_from_height(assetId, ONEDAY(), deleteFromTimestamp.floor(ONEDAY::seconds));

    return *cb;
}

void MarketDB::erase_trades_from_height(AssetId assetId, NonzeroHeight from)
{
    auto table { trades_table(assetId) };
    sqlite::Statement stmt(db, std::format("DELETE FROM {} WHERE height >= ?", table));
    stmt.run(from);
}

void MarketDB::asset_rebuild_candles(AssetId assetId, CandleBegin cb)
{
    // rebuild 5min candles from trades
    std::optional<Candle> c;
    { // rebuild 5 min candle from trades
        Statement stmt(db, std::format("SELECT base, quote FROM {} WHERE height >= ?", trades_table(assetId)));
        stmt.for_each([&](auto& row) {
            double base { row[0] };
            double quote { row[1] };
            auto ta(TradeAmount::create(base, quote));
            assert(ta.has_value()); // should not be zero trade
            if (!c) {
                auto price { ta->price() };
                c = Candle {
                    .timestamp { cb.timestamp },
                    .height { cb.height },
                    .open = price,
                    .high = price,
                    .low = price,
                    .close = price,
                    .base = base,
                    .quote = quote
                };
            } else {
                candle_add_trade(*c, *ta);
            }
        },
            cb.height);
        if (!c.has_value()) {
            // Deleted trades were exactly deleting one candle
            // (no other trades in that candle)
            return;
        }
        insert_candle(assetId, FIVEMIN(), *c);
    }

    auto aggregate_candles_from { [&](Interval inInterval, Interval outInterval) { // now rebuild 1 hour candle from 5 min candles
        auto outBegin = c->timestamp.floor(outInterval.seconds());
        c.reset();
        Statement stmt(db, std::format("SELECT height, open, high, low, close, base, quote FROM {} WHERE timestamp>=?", candles_table(assetId, inInterval)));
        stmt.for_each([&](auto& row) {
            Height height { row[0] };
            double open { row[1] };
            double high { row[2] };
            double low { row[3] };
            double close { row[4] };
            double base { row[5] };
            double quote { row[6] };
            if (!c) {
                c = Candle {
                    .timestamp = outBegin,
                    .height = height,
                    .open = open,
                    .high = high,
                    .low = low,
                    .close = close,
                    .base = base,
                    .quote = quote,
                };
            } else {
                if (c->high < high)
                    c->high = high;
                if (c->low > low)
                    c->low = low;
                c->close = close;
                c->base += base;
                c->quote += quote;
            }
        },
            outBegin);
        assert(c.has_value());
        // now insert into database
        insert_candle(assetId, outInterval, *c);
    } };
    aggregate_candles_from(FIVEMIN(), ONEHOUR());
    aggregate_candles_from(ONEHOUR(), ONEDAY());
}

void MarketDB::asset_rollback(AssetId assetId, NonzeroHeight nextHeight, Timestamp nextTimestamp)
{
    erase_trades_from_height(assetId, nextHeight);

    Statement stmt(db, std::format("SELECT height FROM {} ORDER BY height DESC LIMIT 1", trades_table(assetId)));
    auto latestHeight { stmt.one().process([](auto& row) {
        return Height(row[0]);
    }) };
    stmtSetLatestHeight.run(latestHeight.value_or(Height(0)), assetId);

    if (!latestHeight) { // no more trades present
        asset_drop_tables(assetId);
        return;
    } else { // still trades, we need to rebuild aggregate candles
        assert(latestHeight->is_zero() == false); // trades have nozero height
        auto begin { asset_erase_candles_from(assetId, nextHeight, nextTimestamp) };
        asset_rebuild_candles(assetId, begin);
    }
}

void MarketDB::insert_asset(AssetId assetId, AssetHash hash)
{
    stmtInsertAsset.run(assetId, hash);
}

void MarketDB::insert_candle(AssetId assetId, Interval interval, const Candle& c)
{
    auto table { candles_table(assetId, interval) };
    Statement stmt(db, std::format("INSERT INTO {} (timestamp, height, open, high, low, close, base, quote) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", table));
    stmt.run(c.timestamp, c.height, c.open, c.high, c.low, c.close, c.base, c.quote);
}

wrt::optional<Candle> MarketDB::get_latest_candle(AssetId aid, Interval interval) const
{
    auto tableName { candles_table(aid, interval) };
    std::string query { std::format("SELECT timestamp, height, open, high, low, close, base, quote FROM {} ORDER BY timestamp DESC LIMIT 1", tableName) };
    Statement stmt(db, query);
    return stmt.one().process([](auto& row) { return Candle {
                                                  .timestamp = row[0],
                                                  .height = row[1],
                                                  .open = row[2],
                                                  .high = row[3],
                                                  .low = row[4],
                                                  .close = row[5],
                                                  .base = row[6],
                                                  .quote = row[7],
                                              }; });
}

MarketDB::MarketDB(const std::string& path)
    : MarketReaderDB(create_database(path))
    , stmtSetLatestHeight(db, "UPDATE Assets SET LatestHeight = ? WHERE id = ?")
    , stmtInsertAsset(db, "INSERT INTO Assets (id, hash, latestHeight) VALUES (?, ?, 0)")
    , stmtSelectAssetsFrom(db, "SELECT id, latestHeight FROM Assets WHERE id >= ? ORDER BY id ASC")
    , stmtDeleteAssets(db, "DELETE FROM Assets WHERE id >= ?")
    , stmtSelectAssetIdsByLatestHeight(db, "SELECT id FROM ASSETS WHERE latestHeight >=?")
{
}

void MarketDB::create_tables(AssetId aid)
{
    // create candles tables
    auto create_candles { [&](Interval interval) { db.exec(std::format("CREATE TABLE {} ( `timestamp` INTEGER NOT NULL, `height` INTEGER NOT NULL, `open` REAL NOT NULL, `high` REAL NOT NULL, `low` REAL NOT NULL, `close` REAL NOT NULL, `base` REAL NOT NULL, `quote` REAL NOT NULL, PRIMARY KEY(`timestamp`))", candles_table(aid, interval))); } };
    create_candles(FIVEMIN());
    create_candles(ONEHOUR());
    create_candles(ONEDAY());

    db.exec(std::format("CREATE TABLE {} (`height` INTEGER, `base` REAL NOT NULL, `quote` REAL NOT NULL,  PRIMARY KEY (`height`))", trades_table(aid)));
}

void MarketDB::asset_drop_tables(AssetId aid)
{
    auto drop_table { [&](std::string_view table) {
        db.exec(std::format("DROP TABLE IF EXISTS {}", table));
    } };
    auto drop_candles { [&](Interval interval) { drop_table(candles_table(aid, interval)); } };
    drop_candles(FIVEMIN());
    drop_candles(ONEHOUR());
    drop_candles(ONEDAY());
    drop_table(trades_table(aid));
}

void MarketDB::delete_assets_from(AssetId assetId, NonzeroHeight fromHeight)
{
    stmtSelectAssetsFrom.for_each([&](auto& row) {
        AssetId assetId = row[0];
        Height latestHeight = row[1];
        if (!latestHeight.is_zero()) { // tables for this asset exist
            // There were trades inserted for this assetId.
            // Any trades must have happened after asset was created.
            assert(latestHeight >= fromHeight);

            asset_drop_tables(assetId);
        }
    },
        assetId);
    stmtDeleteAssets.run(assetId);
}
}

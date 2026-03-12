#pragma once
namespace api {
struct Candle;
struct Trade;
template <typename T>
struct ReversibleVector;
using CandlesVector = ReversibleVector<Candle>;
using TradesVector = ReversibleVector<Trade>;
}

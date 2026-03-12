#pragma once
#include "api_types_fwd.hpp"
#include "block/chain/height.hpp"
#include "general/timestamp.hpp"
namespace api {

struct Candle {
    Timestamp timestamp; // begin timestamp
    NonzeroHeight height;
    double open;
    double high;
    double low;
    double close;
    double base;
    double quote;
};

struct Trade {
    Timestamp timestamp;
    NonzeroHeight height;
    double base;
    double quote;
};

template <typename T>
struct ReversibleVector {
    std::vector<T> elements;
    bool reverse = false;
    void foreach (this auto&& self, auto&& lambda) ;
};
}

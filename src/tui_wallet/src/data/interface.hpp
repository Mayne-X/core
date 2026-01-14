#include "api/types/shared.hpp"
#include "data/tuple.hpp"

struct WartBalance : public api::FundsBalance {
    static std::thread get_data(const DataRetrievalContext& ctx, auto callback)
    {
        return std::thread([&ctx, callback = std::move(callback)]() {
            WartBalance bal { ctx.get_wart_balance() };
            callback(std::optional<WartBalance> { bal });
        });
    }
};

using DataInterface = DataTuple<WartBalance>;

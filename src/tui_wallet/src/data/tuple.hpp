#pragma once

#include "data/retrieval_context.hpp"
#include <chrono>
#include <memory>
#include <thread>

template <typename... Ts>
class DataTuple {
    template <typename T>
    struct Entry {
        std::unique_ptr<std::mutex> m;
        std::chrono::steady_clock::time_point expires;
        std::thread t;
        std::optional<T> value;
    };

    template <typename T>
    using TupleEntry = std::unique_ptr<Entry<T>>;
    template <typename T>
    requires(std::is_same_v<T, Ts> || ...)
    void try_update(const DataRetrievalContext& ctx, auto&& on_complete)
    {
        using sc = std::chrono::steady_clock;
        constexpr auto expirationInterval { std::chrono::seconds(5) };
        TupleEntry<T>& r { std::get<TupleEntry<T>>(tuple) };
        std::lock_guard l(r.m);
        if (r.expires <= sc::now()) {
            // start update process
            // set to max while update job is pending
            r.expires = std::chrono::steady_clock::time_point::max();
            // reset value because it expired
            r.value.reset();
            r.t = T::get_data(ctx, [&r, on_complete = std::forward<decltype(on_complete)>(on_complete)](std::optional<T> v) {
                std::lock_guard l(r.m);
                r.expires = sc::now() + expirationInterval;
                r.value = std::move(v);
                on_complete();
            });
        }
    }

public:
    template <typename T>
    requires(std::is_same_v<T, Ts> || ...)
    auto get(const DataRetrievalContext& ctx, auto on_complete) const
    {
        const TupleEntry<T>& r { std::get<TupleEntry<T>>(tuple) };
        try_update<T>(ctx, std::move(on_complete));
        std::lock_guard l(r.m);
        return r.value;
    }
    DataTuple()
        : tuple(std::make_unique<Entry<Ts>>()...)
    {
    }

private:
    std::tuple<TupleEntry<Ts>...> tuple;
};

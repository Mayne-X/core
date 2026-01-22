#pragma once

#include "data/retrieval_context.hpp"
#include <chrono>
#include <memory>
#include <thread>

template <typename... Ts>
class DataStateUpdater {
    template <typename T>
    struct Entry {
        mutable std::mutex m;
        std::atomic<size_t> nonce { 0 };
        std::chrono::steady_clock::time_point expires;
        std::jthread t;
        std::optional<T> value;
    };

    template <typename T>
    using TupleEntry = std::unique_ptr<Entry<T>>;

    template <typename T, typename... Args>
    requires(std::is_same_v<T, Ts> || ...)
    void start_update_locked(Entry<T>& r, auto&& on_complete, Args&&... args)
    {
        using sc = std::chrono::steady_clock;
        // start update process
        // set to max while update job is pending
        r.expires = std::chrono::steady_clock::time_point::max();
        // reset value because it expired
        r.value.reset();
        if (r.t.joinable())
            r.t.join();

        r.nonce += 1;
        r.t = T::get_data(retrievalContext, [&r, n = r.nonce.load(), on_complete = std::forward<decltype(on_complete)>(on_complete)](std::optional<T> v) {
                constexpr auto expirationInterval { std::chrono::seconds(5) };
                std::lock_guard l(r.m);
                if (r.nonce.load() == n) {
                r.expires = sc::now() + expirationInterval;
                r.value = std::move(v);
                on_complete(); 
                } }, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    requires(std::is_same_v<T, Ts> || ...)
    void try_update(bool force, Args&&... args)
    {
        using sc = std::chrono::steady_clock;
        Entry<T>& r { *std::get<TupleEntry<T>>(tuple) };
        std::lock_guard l(r.m);
        if (force || r.expires <= sc::now()) {
            start_update_locked<T>(r, std::forward<Args>(args)...);
        }
    }

public:
    template <typename T, typename... Args>
    requires(std::is_same_v<T, Ts> || ...)
    auto get(bool clearCache, auto on_complete, Args&&... args)
    {
        try_update<T>(clearCache, std::move(on_complete), std::forward<Args>(args)...);
        const Entry<T>& r { *std::get<TupleEntry<T>>(tuple) };
        std::lock_guard l(r.m);
        return r.value;
    }
    DataStateUpdater(DataRetrievalContext retrievalContext)
        : retrievalContext(std::move(retrievalContext))
        , tuple(std::make_unique<Entry<Ts>>()...)
    {
    }

    DataRetrievalContext retrievalContext;

private:
    std::tuple<TupleEntry<Ts>...> tuple;
};

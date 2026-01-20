#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

template <typename T>
struct Channel {
private:
    std::mutex m;
    std::vector<T> pendingMessages;
    std::vector<T> collectedMessages;
    std::size_t maxSize { 200 };
    std::size_t pruneTolerance { 40 };

public:
    void push_back(T msg)
    {
        std::lock_guard l(m);
        pendingMessages.push_back(std::move(msg));
    }
    [[nodiscard]] const std::vector<T>& messages()
    {
        std::vector<T> v;
        {
            std::lock_guard l(m);
            v = std::move(pendingMessages);
            pendingMessages.clear();
        }
        collectedMessages.insert(collectedMessages.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
        prune();
        return collectedMessages;
    }
    void prune()
    {
        if (collectedMessages.size() > maxSize + pruneTolerance)
            collectedMessages.erase(collectedMessages.begin(), collectedMessages.begin() + collectedMessages.size() - maxSize);
    }
};

using LogChannel = Channel<std::string>;

struct RequestLogEntry {
    using clock = std::chrono::steady_clock;

private:
    clock::time_point begin;
    std::atomic<clock::time_point> end;
    std::atomic<std::optional<bool>> success;
    std::string msg;

public:
    RequestLogEntry(std::string endpoint)
        : begin(clock::now())
        , end {}
        , msg(std::move(endpoint))
    {
    }
    auto& message() const { return msg; }
    void set_success(bool succeeded)
    {
        end = clock::now();
        success = succeeded;
    }
    struct State {
        clock::duration elapsed;
        std::optional<bool> success;
    };
    State state() const
    {
        auto e { end.load() };
        if (e == clock::time_point {}) { // still pending;
            return { clock::now() - begin, success };
        } else {
            return { e - begin, success };
        }
    }
};

using RequestLogChannel = Channel<std::shared_ptr<RequestLogEntry>>;

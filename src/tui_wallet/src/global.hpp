#pragma once
#include "data/interface.hpp"
#include "log_channel.hpp"
// #include "log_lines.hpp"
#include <memory>
namespace global {
struct Globals {
    DataInterface dataInterface;
    // LogLines log;
    LogChannel logChannel;
    RequestLogChannel requestLogChannel;
    Globals(DataRetrievalContext init)
        : dataInterface(std::move(init))
    {
    }
};

inline std::unique_ptr<Globals> ptr;

inline void init(DataRetrievalContext g)
{
    ptr = std::make_unique<Globals>(std::move(g));
}
inline auto& globals() { return *ptr; }
inline auto& data_interface() { return ptr->dataInterface; }
inline auto& endpoint() { return data_interface().retrievalContext.endpoint; }
inline auto& log() { return globals().logChannel; }
inline auto& request_log() { return globals().requestLogChannel; }
}

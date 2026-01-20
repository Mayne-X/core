#include "global.hpp"
void log(std::string_view msg)
{
    global::log().push_back(std::string(msg));
};

std::weak_ptr<RequestLogEntry> log_request(std::string msg)
{
    auto p { std::make_shared<RequestLogEntry>(std::move(msg)) };
    std::weak_ptr<RequestLogEntry> w { p };
    global::request_log().push_back(std::move(p));
    return w;
}

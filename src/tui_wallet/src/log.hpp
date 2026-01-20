#pragma once

#include "log_channel.hpp"
#include <memory>
#include <string_view>
void log(std::string_view msg);
[[nodiscard]] std::weak_ptr<RequestLogEntry> log_request(std::string msg);

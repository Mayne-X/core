#pragma once
#include <chrono>
#include <string>
[[nodiscard]] std::string format_duration(std::chrono::steady_clock::duration d);

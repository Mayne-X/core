#include "time_format.hpp"
using namespace std::chrono;
using namespace std::chrono_literals;
std::string format_duration(std::chrono::steady_clock::duration d)
{
    size_t mins { 0 };
    if (d > 1min) {
        auto m { duration_cast<minutes>(d) };
        d -= m;
        mins = m.count();
    }
    size_t secs { 0 };
    if (d > 1s) {
        auto s { duration_cast<seconds>(d) };
        d -= s;
        secs = s.count();
    }
    if (mins != 0) {
        return std::to_string(mins) + "min + " + std::to_string(secs) + "s";
    }
    if (secs != 0) {
        return std::to_string(secs) + "s";
    }
    // return milliseconds
    auto ms { duration_cast<milliseconds>(d).count() };
    return std::to_string(ms) + "ms";
}

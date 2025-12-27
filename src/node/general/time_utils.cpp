#include "time_utils.hpp"
#include "wrt/format.hpp"
namespace timing {

std::string Duration::format()
{
    auto ms{milliseconds()};
    if (ms <1000) 
        return fmt_lib::format("{}ms",ms);
    auto d{double(ms)/1000};
    if (d<60) 
        return fmt_lib::format("{:.1f}s",d);
    auto s_total{ms/(60*1000)};
    auto min{s_total/60};
    auto s{s_total % 60};
    return fmt_lib::format("{}min {}s",min,s);
};

}

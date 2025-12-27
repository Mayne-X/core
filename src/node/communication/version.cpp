#include "version.hpp"
#include "spdlog/common.h"
#include <string>
namespace fmt = spdlog::fmt_lib;

std::string NodeVersion::to_string() const
{
    return fmt::format("v{}.{}.{}", major(), minor(), patch());
}

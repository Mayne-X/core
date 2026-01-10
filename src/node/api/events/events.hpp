#pragma once
#include "api/types/shared.hpp"
namespace api {

namespace event {
    using Event = std::variant<Rollback, Block>;
}
using Event = event::Event;
}


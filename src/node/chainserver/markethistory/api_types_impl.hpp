#pragma once
#include "api_types.hpp"
#include <ranges>

namespace api {
template <typename T>
void ReversibleVector<T>::foreach (this auto&& self, auto&& lambda)
{
    if (self.reverse) {
        for (auto& e : std::views::reverse(std::forward<decltype(self)>(self).elements)) {
            std::forward<decltype(lambda)>(lambda)(e);
        }
    } else {
        for (auto& e : std::forward<decltype(self)>(self).elements) {
            std::forward<decltype(lambda)>(lambda)(e);
        }
    }
}
}

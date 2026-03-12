#pragma once
#include <optional>
template <typename T>
struct OptParam : public std::optional<T> {
    explicit OptParam(std::optional<T> o)
        : std::optional<T>(std::move(o))
    {
    }
};

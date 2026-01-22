#pragma once
#include "glaze/glaze.hpp"
namespace glz {
template <read_supported<JSON> T, is_buffer Buffer>
requires(!is_input_streaming<std::remove_reference_t<Buffer>>)
[[nodiscard]] expected<T, error_ctx> read_json_ignore(Buffer&& buffer)
{
    T value {};
    context ctx {};
    const error_ctx ec = read<opts { .error_on_unknown_keys = false, .error_on_missing_keys = true }>(value, std::forward<Buffer>(buffer), ctx);
    if (ec) {
        return unexpected<error_ctx>(ec);
    }
    return value;
}
}

template <typename Data>
struct API_Return {
    int code;
    std::optional<std::string> error;
    std::optional<Data> data;
};

template <typename T>
[[nodiscard]] T parse_api(std::string_view jsonStr)
{
    auto p { glz::read_json_ignore<API_Return<T>>(jsonStr) };
    if (p.has_value()) {
        API_Return<T>& v { *p };
        if (v.error)
            throw std::runtime_error(std::format("API error: {}, (code {})", *v.error, v.code));
        if (!v.data)
            throw std::runtime_error(std::format("API didn't return any data, code {}", v.code));
        return std::move(*v.data);
    }
    throw std::runtime_error(std::format("API JSON Error: {}", glz::format_error(p.error(), jsonStr)));
}

struct Parse {
    Parse(std::string_view jsonStr)
        : jsonStr(jsonStr)
    {
    }
    std::string_view jsonStr;
    template <typename T>
    operator T() &&
    {
        return parse_api<T>(jsonStr);
    }
};

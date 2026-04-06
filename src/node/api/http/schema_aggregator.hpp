#pragma once
#include "glaze/json/schema.hpp"
#include <map>
#include <string_view>
struct SchemaAggregator {
private:
    std::map<std::string_view, glz::detail::schematic, std::less<>> defs;

public:
    template <typename T>
    void add_type()
    {
        auto& def = defs[glz::name_v<T>];
        if (!def.type) {
            glz::detail::to_json_schema<std::decay_t<T>>::template op<glz::opts {}>(def, defs);
        }
    }
    auto to_string() const
    {
        constexpr glz::opts Opts;
        static constexpr glz::opts options = glz::opts_write_type_info_off<decltype(Opts)> { { Opts } };
        std::string buf;
        auto res { glz::write<options>(std::move(defs), buf) };
        assert(!res);
        return buf;
    }
};

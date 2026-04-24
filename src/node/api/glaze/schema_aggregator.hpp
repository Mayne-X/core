#pragma once
#include "api/reply.hpp"
#include "glaze/json/schema.hpp"
#include <map>
#include <string>
struct SchemaAggregator {
private:
    std::map<std::string_view, glz::schema, std::less<>> defs;
    std::map<std::string_view, size_t> ref_counts;

public:
    template <typename T>
    std::string_view add_type()
    {
        auto name { glz::name_v<T> };
        ref_counts[name] += 2; // to make the original types appear in the final list they need ref_count >= 2
        auto& def = defs[name];
        if (!def.type) {
            glz::detail::to_json_schema<std::decay_t<T>>::template op<glz::opts {}>(def, defs);
        }
        return name;
    }
    HTMLString to_html_list() const;
    void inline_by_refcount()
    {

        // Inline single-use $defs entries at their reference sites
        for (auto& [name, def] : defs) {
            glz::detail::count_schema_refs(def, ref_counts);
        }

        // First inline refs within defs entries (for chained single-use types)
        for (auto& [_, def] : defs) {
            glz::detail::inline_single_use_refs(def, defs, ref_counts);
        }
        // Then inline refs in the main schema tree
        glz::detail::prune_inlined_defs(defs, ref_counts);
    }
    JSONString to_string() const
    {
        constexpr glz::opts Opts;
        static constexpr glz::opts options = glz::opts_write_type_info_off<decltype(Opts)> { { Opts } };
        std::string buf;
        auto res { glz::write<options>(defs, buf) };
        assert(!res);
        return buf;
    }
};

#include "asset.hpp"
#include "general/reader.hpp"
#include "general/view.hpp"
AssetName::AssetName(View<maxlen> data)
    : AssetName([&data]() {
        size_t end { data.size() };
        for (size_t i = 0; i < maxlen; ++i) {
            auto c { data[i] };
            if (end == data.size()) {
                // no zero byte found yet
                if (c == 0)
                    end = i;
            } else {
                // zero byte was found before
                // now only zero bytes can follow
                if (c != 0)
                    throw Error(EASSETNAME);
            }
        }
        return try_parse(std::string((char*)data.data(), end))
            .value_or_throw();
    }())
{
}

AssetName::AssetName(Reader& r)
    : AssetName(r.view<5>()) { };

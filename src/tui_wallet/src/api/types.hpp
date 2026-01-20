#pragma once

#include <cstdint>
#include <string>
#include <vector>
struct api_types {
    struct TokenListEntry {
        std::string hash;
        uint32_t height;
        std::string name;
    };
    struct TokenList {
        TokenList(std::string prefix)
            : prefix(std::move(prefix))
        {
        }
        std::vector<TokenListEntry> entries;
        std::string prefix;
    };
};

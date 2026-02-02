#pragma once

#include <cstdint>
#include <string>
#include <vector>
struct api_types {
    struct TokenListEntry {
        std::string hash;
        uint32_t height;
        std::string name;
        int precision;
    };
    struct TokenList {
        TokenList(std::string namePrefix, std::string hashPrefix)
            : namePrefix(std::move(namePrefix))
            , hashPrefix(std::move(hashPrefix))
        {
        }
        std::vector<TokenListEntry> entries;
        std::string namePrefix;
        std::string hashPrefix;
    };
};

#pragma once
#include "block/body/body_fwd.hpp"
#include "crypto/hash.hpp"
struct MerkleLeaves {
    void add_hash(Hash hash)
    {
        hashes.push_back(std::move(hash));
    }
    std::vector<uint8_t> merkle_prefix() const; // only since shifus merkle tree
    Hash merkle_root(const BodyData& data, NonzeroHeight h) const;
    std::vector<Hash> hashes;
};

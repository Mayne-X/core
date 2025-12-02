#pragma once

#include "crypto/hasher_sha256.hpp"
#include "general/merkle_leaves.hpp"
#include "general/writer.hpp"
struct MerkleWriteHooker;
struct MerkleWriteHook {
public:
    operator Writer&()
    {
        return writer;
    }

    MerkleWriteHook(MerkleWriteHook&& mi)
        : MerkleWriteHook(mi.writer, mi.creator)
    {
        mi.begin = nullptr;
    }

    ~MerkleWriteHook();

private:
    friend MerkleWriteHooker;
    MerkleWriteHook(Writer& w, MerkleWriteHooker& c);
    MerkleWriteHook(const MerkleWriteHook& mi) = delete;

public:
    Writer& writer;

private:
    MerkleWriteHooker& creator;
    const uint8_t* begin;
};

namespace merkle_write{

struct ByteCounter {

    ::ByteCounter writer;
};

}

struct MerkleWriteHooker {
    friend MerkleWriteHook;
    MerkleWriteHooker(Writer& w)
        : writer(w)
    {
    }
    operator MerkleWriteHook()
    {
        return hook();
    }
    [[nodiscard]] MerkleWriteHook hook();
    MerkleLeaves move_leaves() && { return std::move(leaves); }

private:
    void add_hash_of(const std::span<const uint8_t>& s)
    {
        leaves.add_hash(hashSHA256(s));
    }

public:
    Writer& writer;

private:
    MerkleLeaves leaves;
};

#include "merkle_write_hooker.hpp"
MerkleWriteHook::MerkleWriteHook(Writer& w, MerkleWriteHooker& c)
    : writer(w)
    , creator(c)
    , begin(w.cursor())
{
}

MerkleWriteHook::~MerkleWriteHook()
{
    if (begin)
        creator.add_hash_of({ begin, writer.cursor() });
}

MerkleWriteHook MerkleWriteHooker::hook()
{
    return { writer, *this };
}

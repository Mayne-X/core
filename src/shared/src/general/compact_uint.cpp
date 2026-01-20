#include "compact_uint.hpp"
#include "general/reader.hpp"
CompactUInt::CompactUInt(Reader& r)
    :CompactUInt(from_value_throw(r.uint16()))
{
}

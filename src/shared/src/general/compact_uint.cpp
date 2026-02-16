#include "compact_uint.hpp"
#include "general/reader.hpp"
CompactUInt::CompactUInt(Reader& r)
    : CompactUInt(from_value_throw(r.uint16()))
{
}

Result<CompactUInt> CompactUInt::try_parse(std::string_view s, bool ceil)
{
    auto w { Wart::try_parse(s) };
    if (w) {
        return compact(*w, ceil);
    }
    return Error(EBADFEE);
}

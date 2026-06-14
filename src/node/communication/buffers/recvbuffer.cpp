#include "recvbuffer.hpp"
#include "communication/messages_impl.hpp"
#include "crypto/hasher_sha256.hpp"
#include "general/hex.hpp"
#include <utility>

bool Rcvbuffer::verify()
{
    auto h = hashSHA256(body.bytes);
    if (memcmp(header + 4, h.data(), 4) != 0) {
        return false;
    };
    return true;
}
namespace {

template <typename V, uint8_t prevcode>
V check(uint8_t, Reader&)
{
    throw Error(EMSGTYPE);
}
template <typename V, uint8_t prevcode, typename T, typename... S>
V check(uint8_t type, Reader& r)
{
    // variant types must be in order and message codes must be all different
    static_assert(prevcode < T::msgcode);
    if (T::msgcode == type)
        return V { std::in_place_type<T>, r };
    return check<V, T::msgcode, S...>(type, r);
}

template <typename V, typename T, typename... S>
V check_first(uint8_t type, Reader& r)
{
    if (T::msgcode == type)
        return V { std::in_place_type<T>, r };
        // return T(r);
    return check<V, T::msgcode, S...>(type, r);
}

// do metaprogramming dance
template <typename T>
class VariantParser {
};

template <typename... Types>
class VariantParser<std::variant<Types...>> {
public:
    static auto parse(uint8_t type, Reader& r)
    {
        using ret_t = std::variant<Types...>;
        auto c{r.cursor()};
        auto n { r.remaining() };
        try {
            auto res { check_first<ret_t, Types...>(type, r) };
            if (r.remaining() != 0)
                throw Error(EMSGINTEGRITY1);
            return res;
        } catch (...) {
            auto hex {serialize_hex(std::span{c,n})};
            spdlog::error("Cannot parse message of type {} with {} bytes: {}", type, n, hex);
            throw;
        }
    }
};

}

msg::Msg Rcvbuffer::parse()
{
    if (!verify())
        throw Error(ECHECKSUM);
    using namespace msg;
    Reader r(*this);
    return VariantParser<msg::Msg>::parse(type(), r);
}

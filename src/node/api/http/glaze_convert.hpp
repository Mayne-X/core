#pragma once
// #include "api/events/subscription.hpp"
// #include "api/interface.hpp"
#include "api/types/all.hpp"
#include "block/header/batch.hpp"
#include "glaze_types.hpp"
#include "peerserver/db/peer_db.hpp"
namespace api {
namespace glaze {

Grid from(const ::Grid&);
Hash from(const ::Hash&);
Wart from(const ::Wart);
BanEntry from(const ::PeerDB::BanEntry&);
Account from(const ::api::Account&);
AssetBasic from(const ::AssetBasic&);
ThrottledPeer from(const ::api::ThrottledPeer&);
Token from(const ::api::Token&);
TokenBalanceLookup from(const ::api::TokenBalanceLookup&);
AssetDetail from(const ::AssetDetail&);
uint64_t from(const IsUint64& i);
uint32_t from(const IsUint32& i);
uint32_t from(const TokenDecimals& d);
TransactionId from(const ::TransactionId);
AssetLookupTrace from(const ::api::AssetLookupTrace& a);
TransactionDetails to_json(const api::TransactionDetails&);
TransactionSignedCommon from(const api::block::TransactionSignedData&);

template <typename T>
using target_type = std::remove_cvref_t<decltype(from(std::declval<T>()))>;

}

}

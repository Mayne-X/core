#pragma once
#include "../../types/peer_requests.hpp"
struct ProbeData;
struct ProbeBalanced {

    [[nodiscard]] static std::optional<Batchrequest> slot_batch_request(const ProbeData&, const std::shared_ptr<Descripted>&, Batchslot s, Header h);
    [[nodiscard]] static std::optional<Batchrequest> final_partial_batch_request(const ProbeData&, const std::shared_ptr<Descripted>&, NonzeroHeight maxLength, Worksum minWork);
    [[nodiscard]] static std::optional<Proberequest> probe_request(const ProbeData&, const std::shared_ptr<Descripted>&, NonzeroHeight maxLength);

    // This variable contains a pinned chain we know and the fork range
    // where the peer's advertised chain forks from our known chain
    const ProbeData& probeData;

    // This is the maximal chain length of headers we want to sync from the peer.
    NonzeroHeight maxLength;
};

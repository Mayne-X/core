#pragma once

#include "all_fwd.hpp"
#include "block/chain/height.hpp"
#include "eventloop/types/conndata.hpp"
#include "peerserver/db/offense_entry.hpp"
namespace api {

using sc = std::chrono::steady_clock;
struct ThrottleState {
    struct BatchThrottler {
        Height h0;
        Height h1;
        size_t window;
        template <size_t w>
        BatchThrottler(const BatchreqThrottler<w>& bt)
            : h0(bt.h0())
            , h1(bt.h1())
            , window(w)
        {
        }
    };
    sc::duration delay;
    BatchThrottler batchreq;
    BatchThrottler blockreq;
    ThrottleState(const ThrottleQueue& t)
        : delay(t.reply_delay())
        , batchreq(t.headerreq)
        , blockreq(t.blockreq)
    {
    }
};
struct Peerinfo {
    Peeraddr endpoint;
    uint64_t id;
    bool initialized;
    PeerChain chainstate;
    SignedSnapshot::Priority theirSnapshotPriority;
    SignedSnapshot::Priority acknowledgedSnapshotPriority;
    uint32_t since;
    ThrottleState throttle;
};
struct ThrottledPeer {
    Peeraddr endpoint;
    uint64_t id;
    ThrottleState throttle;
};
struct PeerinfoConnections {
    const std::vector<api::Peerinfo>& v;
    static constexpr auto map = [](const Peerinfo& pi) -> auto& { return pi.endpoint; };
};
struct TCPConnectionSchedule {
    struct Schedule {
        TCPPeeraddr address;
        Error lastError;
        uint32_t sleepDuration;
        std::optional<uint32_t> expiresIn;
    };
    struct VerifiedSchedule {
        uint32_t lastVerified;
        Schedule schedule;
    };
    std::vector<VerifiedSchedule> connectedVerified;
    std::vector<VerifiedSchedule> disconnectedVerified;
    std::vector<Schedule> feelers;
};
struct WSConnectionSchedule {
};

// struct MempoolEntry : public TransactionMessage {
//     TxHash txHash;
// };

using OffenseEntry = ::OffenseEntry;
}

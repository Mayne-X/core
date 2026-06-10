#include "probe_balanced.hpp"
#include "../../types/probe_data.hpp"

namespace {
struct ProbeRange {
    [[nodiscard]] ProbeRange(const ProbeBalanced& pb)
        : ProbeRange {
            [&pb] {
                auto l { compute_lower(pb.probeData) };
                assert(pb.maxLength + 1 >= l);
                auto u { compute_upper(pb.probeData, pb.maxLength) };
                assert(u >= l);
                auto width { u - l };
                return ProbeRange {
                    l,
                    width,
                    pb.maxLength
                };
            }()
        }
    {
    }

    auto max_length() const { return _maxLength; }
    auto lower() const { return _lower; }
    auto midpoint() const { return _lower + _width / 2; }
    bool can_download() const
    {
        auto downloadLength = _maxLength + 1 - _lower;
        assert(downloadLength > 0); // otherwise the batch would be shared
        return (downloadLength < 20 || _width * 2 < downloadLength);
    }

private:
    static NonzeroHeight compute_lower(const ProbeData& probeData)
    {
        return probeData.fork_range().lower();
    }
    static NonzeroHeight compute_upper(const ProbeData& probeData, Height maxLength)
    {
        assert(probeData.headers()->length() + 1 >= probeData.fork_range().lower()); // TODO: this fails for some people, must be a bug somewhere
        auto u { (std::min(probeData.headers()->length(), maxLength) + 1).nonzero_assert() };
        if (auto& fr = probeData.fork_range(); fr.forked() && fr.upper() < u)
            u = fr.upper();
        return u;
    }
    ProbeRange(NonzeroHeight lower, uint32_t width, NonzeroHeight maxLength)
        : _lower(lower)
        , _width(width)
        , _maxLength(maxLength)
    {
        assert(_lower + _width <= maxLength + 1); // guarranteed compute_upper()
    }
    NonzeroHeight _lower;
    uint32_t _width;
    NonzeroHeight _maxLength;
};
}

std::optional<Proberequest>
ProbeBalanced::probe_request(const ProbeData& pd, const std::shared_ptr<Descripted>& desc, NonzeroHeight maxLength)
{
    ProbeRange pr({ pd, maxLength });
    if (!pr.can_download())
        return Proberequest(desc, pr.midpoint());

    return {};
}

[[nodiscard]] std::optional<HeaderRequest> ProbeBalanced::slot_batch_request(const ProbeData& pd, const std::shared_ptr<Descripted>& desc, Batchslot slot, Header h)
{
    ProbeRange pr({ pd, slot.upper() });
    if (pr.can_download()) {
        return HeaderRequest(desc, pd.headers(), HeaderRange { slot.lower(), pr.max_length() + 1 }, h);
        // TODO max_length() without +1 ?
        return HeaderRequest(desc, pd.headers(), HeaderRange { slot.lower(), pr.max_length() }, h);
    }
    return {};
}

[[nodiscard]] std::optional<HeaderRequest> ProbeBalanced::final_partial_batch_request(const ProbeData& pd, const std::shared_ptr<Descripted>& desc, NonzeroHeight maxLength, Worksum minWork)
{
    Batchslot slot(maxLength);
    if (slot.upper() == maxLength)
        return {};
    ProbeRange pr({ pd, maxLength });

    if (pr.can_download()) {
        if (slot.lower() > pr.lower()) {
            return HeaderRequest(desc, slot, maxLength - slot.offset(), minWork);
        } else {
            return HeaderRequest(desc, pd.headers(), HeaderRange { pr.lower(), maxLength + 1 }, minWork);
        }
    }
    return {};
}

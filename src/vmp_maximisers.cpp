#include <vmp_maximisers.h>

namespace vmp
{

Packing maximiseByLocalSearch(
    const GeneralInstance &instance, const size_t allowedHostCount,
    Host (*localMaximiser)(GuestProfitVecIt, GuestProfitVecIt, size_t),
    const double localApproximationRatio, const double epsilon)
{
    std::vector initialPlacements(instance.guests.size(), false);

    std::vector<std::shared_ptr<Host>> hosts;
    hosts.reserve(allowedHostCount);

    for (size_t i = 0; i < allowedHostCount; ++i) {
        const auto host = std::make_shared<Host>(instance.capacity);

        for (size_t j = 0; j < instance.guests.size(); ++j) {
            if (!initialPlacements[j] &&
                host->accommodatesGuest(*instance.guests[j])) {
                host->addGuest(instance.guests[j]);
                initialPlacements[j] = true;
            }
        }

        hosts.push_back(host);
    }

    const size_t iterations = std::abs(static_cast<int>(std::ceil(
        1.0 / localApproximationRatio * static_cast<double>(allowedHostCount) *
        std::log(1.0 / epsilon))));

    for (size_t _ = 0; _ < iterations; ++_) {
        int maxImprovement = 0;
        size_t mostImprovableIndex = 0;
        std::shared_ptr<Host> mostImprovingCandidate;

        for (size_t i = 0; i < hosts.size(); i++) {
            std::vector<std::pair<std::shared_ptr<Guest>, int>>
                guestsWithValues;

            for (const auto &guest : instance.guests) {
                int value = 1;
                for (const auto &host : hosts) {
                    if (host != hosts[i] && host->hasGuest(guest)) {
                        value = 0;
                        break;
                    }
                }
                guestsWithValues.emplace_back(guest, value);
            }

            Host candidate =
                localMaximiser(guestsWithValues.begin(),
                               guestsWithValues.end(), instance.capacity);
            const int improvement =
                candidate.guestCount() - hosts[i]->guestCount();

            if (improvement > maxImprovement) {
                maxImprovement = improvement;
                mostImprovableIndex = i;
                mostImprovingCandidate = std::make_shared<Host>(candidate);
            }
        }

        if (maxImprovement <= 0) {
            break;
        }

        hosts[mostImprovableIndex] = mostImprovingCandidate;
        for (const auto &host : hosts) {
            if (host == hosts[mostImprovableIndex]) {
                continue;
            }

            for (const auto &guest : hosts[mostImprovableIndex]->guests)
                if (host->hasGuest(guest)) {
                    host->removeGuest(guest);
                }
        }
    }

    size_t count = 0;
    for (const auto &host : hosts) {
        count += host->guestCount();
    }

    // Clean up as there is no guarantee that we will utilise all bins
    std::erase_if(hosts, [](const std::shared_ptr<Host> &host) {
        return host->guestCount() == 0;
    });
    return Packing(hosts);
}
}  // namespace vmp
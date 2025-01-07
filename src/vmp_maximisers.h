#ifndef VMP_MAXIMISERS_H
#define VMP_MAXIMISERS_H

#include <vmp_iterators.h>
#include <vmp_packing.h>

namespace vmp
{

template <PairIterator<std::shared_ptr<Guest>, int> GuestProfitIt>
Packing maximiseByLocalSearch(
    const GeneralInstance &instance, const size_t allowedHosts,
    std::function<Host(GuestProfitIt, GuestProfitIt, size_t)> localMaximiser,
    const double localMaximiserBeta, const double epsilon)
{
    std::vector<std::shared_ptr<Host>> hosts;
    hosts.reserve(allowedHosts);
    for (size_t i = 0; i < allowedHosts; i++) {
        hosts.push_back(std::make_shared<Host>(instance.capacity));
    }

    const size_t iterations = std::ceil(
        1.0 / (localMaximiserBeta * static_cast<double>(allowedHosts)) *
        std::log(1.0 / epsilon));

    for (size_t _ = 0; _ < iterations; ++_) {
        size_t maxImprovement = 0;
        size_t mostImprovableIndex = 0;
        std::shared_ptr<Host> mostImprovingCandidate;

        for (size_t i = 0; i < hosts.size(); i++) {
            std::vector<std::pair<std::shared_ptr<Guest>, int>>
                guestsWithValues;

            for (const auto &guest : instance.guests) {
                int value = 0;
                for (const auto &host : hosts) {
                    if (host != hosts[i] && host->hasGuest(guest)) {
                        value = 1;
                        break;
                    }
                }
                guestsWithValues.emplace_back(guest, value);
            }

            Host candidate =
                localMaximiser(guestsWithValues.begin(),
                               guestsWithValues.end(), instance.capacity);

            const size_t improvement =
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
    }

    return Packing(hosts);
}
}  // namespace vmp

#endif  // VMP_MAXIMISERS_H

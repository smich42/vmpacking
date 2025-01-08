#ifndef VMP_MAXIMISERS_H
#define VMP_MAXIMISERS_H

#include <numeric>
#include <vmp_iterators.h>
#include <vmp_packing.h>

namespace vmp
{

static std::optional<std::vector<std::pair<std::shared_ptr<Guest>, int>>>
findMostEfficientCluster(const std::map<std::shared_ptr<Guest>, int> &unplaced,
                         const Host &host, const int clusterSize)
{
    std::vector<bool> selector(unplaced.size());
    std::fill(selector.end() - clusterSize, selector.end(), true);

    std::optional<std::vector<std::pair<std::shared_ptr<Guest>, int>>>
        bestCluster;
    double bestClusterValue = 0.;

    do {
        std::vector<std::pair<std::shared_ptr<Guest>, int>> candidateSet;

        auto it = unplaced.begin();
        for (size_t i = 0; i < unplaced.size(); ++i, ++it) {
            if (selector[i]) {
                candidateSet.emplace_back(*it);
            }
        }

        if (!host.accommodatesGuests(candidateSet.begin(),
                                     candidateSet.end())) {
            continue;
        }

        const double rewardSum =
            std::accumulate(candidateSet.begin(), candidateSet.end(), 0.,
                            [](const double acc, const auto &guest) {
                                return acc + guest.second;
                            });
        const double clusterValue =
            rewardSum / static_cast<double>(
                            1 + host.countPagesWithGuests(candidateSet.begin(),
                                                          candidateSet.end()));

        if (clusterValue > bestClusterValue) {
            bestCluster = std::move(candidateSet);
            bestClusterValue = clusterValue;
        }
    } while (std::next_permutation(selector.begin(), selector.end()));

    return bestCluster;
}

template <typename GuestProfitIt, typename K = std::shared_ptr<Guest>,
          typename V = int>
    requires PairIterator<GuestProfitIt, K, V>
Host maximiseSingleHostBySimpleEfficiency(GuestProfitIt guestsBegin,
                                          GuestProfitIt guestsEnd,
                                          const size_t capacity,
                                          int initialClusterSize = 1)
{
    Host host(capacity);
    std::map unplaced(guestsBegin, guestsEnd);

    while (true) {
        auto bestGuestSet =
            findMostEfficientCluster(unplaced, host, initialClusterSize);

        while (!bestGuestSet.has_value() && initialClusterSize > 0) {
            --initialClusterSize;
            bestGuestSet =
                findMostEfficientCluster(unplaced, host, initialClusterSize);
        }

        if (!bestGuestSet.has_value()) {
            break;
        }

        for (const auto &guest : bestGuestSet.value()) {
            unplaced.erase(guest.first);
            host.addGuest(guest.first);
        }
    }

    return host;
}

Packing maximiseByLocalSearch(const GeneralInstance &instance,
                              size_t allowedHostCount,
                              Host (*localMaximiser)(GuestProfitVecIt,
                                                     GuestProfitVecIt, size_t),
                              double localApproximationRatio, double epsilon);

}  // namespace vmp

#endif  // VMP_MAXIMISERS_H

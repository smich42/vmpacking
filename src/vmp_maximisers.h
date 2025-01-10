#ifndef VMP_MAXIMISERS_H
#define VMP_MAXIMISERS_H

#include <numeric>
#include <vmp_iterators.h>
#include <vmp_packing.h>

namespace vmp
{

/**
 * Finds the most efficient cluster of guests to place on a host given a
 * mandatory cluster size, accounting for the reward and page sharing within
 * the cluster and with the host.
 *
 * @param unplaced the pool of guests to sample
 * @param host the host to place the guests on
 * @param clusterSize the number of guests to place
 * @return the most efficient cluster of guests, or std::nullopt if no valid
 */
static std::optional<std::vector<std::pair<std::shared_ptr<const Guest>, int>>>
findMaxValueCluster(
    const std::map<std::shared_ptr<const Guest>, int> &unplaced,
    const Host &host, const int clusterSize)
{
    // TODO: parameterise the value function?

    std::vector<bool> selector(unplaced.size());
    std::fill(selector.end() - clusterSize, selector.end(), true);

    std::optional<std::vector<std::pair<std::shared_ptr<const Guest>, int>>>
        bestCluster = std::nullopt;
    double bestClusterValue = 0.;

    do {
        std::vector<std::pair<std::shared_ptr<const Guest>, int>> candidateSet;

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

/**
 * Places guests on a single host by always picking the next most valuable set
 * based on the reward and page sharing. See Li, et al. (2009) and Rampersaud &
 * Grosu (2014).
 *
 * @param guestsBegin the start of the guest range
 * @param guestsEnd the end of the guest range
 * @param capacity the fixed host capacity
 * @param initialClusterSize the initial cluster size to try. Note that the
 * algorithm is little-o(n^clusterSize) where n is the number of guests.
 * Defaults to 1.
 * @return a host with the most valuable guests placed
 */
template <typename GuestProfitIt, typename K = std::shared_ptr<const Guest>,
          typename V = int>
    requires PairIterator<GuestProfitIt, K, V>
Host maximiseOneHostByClusterValues(GuestProfitIt guestsBegin,
                                    GuestProfitIt guestsEnd,
                                    const size_t capacity,
                                    int initialClusterSize = 1)
{
    // TODO: parameterise the value function?

    Host host(capacity);
    std::map unplaced(guestsBegin, guestsEnd);

    while (true) {
        auto bestGuestSet =
            findMaxValueCluster(unplaced, host, initialClusterSize);

        while (!bestGuestSet.has_value() && initialClusterSize > 0) {
            --initialClusterSize;
            bestGuestSet =
                findMaxValueCluster(unplaced, host, initialClusterSize);
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

/**
 * Maximises the number of guests placed on `allowedHostCount` hosts by using a
 * single-host maximiser. See Fleischer, et al. (2006).
 *
 * @param instance the instance to maximise
 * @param allowedHostCount the number of hosts to use
 * @param oneHostMaximiser the single-host maximiser to use
 * @param oneHostApproxRatio the approximation ratio of the single-host
 * maximiser
 * @param epsilon the approximation factor from Fleischer, et al. (2006). The
 * resulting approximation factor is beta/(beta + 1) - epsilon.
 * @return
 */
Packing maximiseByLocalSearch(
    const GeneralInstance &instance, size_t allowedHostCount,
    Host (*oneHostMaximiser)(GuestProfitVecIt, GuestProfitVecIt, size_t),
    double oneHostApproxRatio, double epsilon);

}  // namespace vmp

#endif  // VMP_MAXIMISERS_H

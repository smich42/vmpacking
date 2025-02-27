#ifndef VMP_MAXIMISERS_H
#define VMP_MAXIMISERS_H

#include <vmp_clustertreeinstance.h>
#include <vmp_packing.h>
#include <vmp_types.h>

#include <cassert>
#include <iostream>
#include <numeric>
#include <ranges>

namespace vmp
{

/**
 * Finds the most efficient subset of guests to place on a host given a
 * mandatory subset size, accounting for the reward and page sharing within
 * the subset and with the host.
 *
 * @param unplaced the pool of guests to sample
 * @param host the host to place the guests on
 * @param subsetSize the number of guests to place
 * @return the most efficient subset of guests, or std::nullopt if no valid
 */
static std::optional<std::vector<std::pair<std::shared_ptr<const Guest>, int>>>
findMostEfficientSubset(const std::unordered_map<std::shared_ptr<const Guest>, int> &unplaced,
                        const Host &host, const int subsetSize)
{
    // We do not use bitmask here as we expect to go over all guests
    std::vector<bool> selector(unplaced.size());
    std::fill(selector.end() - subsetSize, selector.end(), true);

    std::optional<std::vector<std::pair<std::shared_ptr<const Guest>, int>>> bestSubset =
        std::nullopt;
    double bestSubsetValue = 0.;

    do {
        std::vector<std::pair<std::shared_ptr<const Guest>, int>> candidateSet;

        auto it = unplaced.begin();
        for (size_t i = 0; i < unplaced.size(); ++i, ++it) {
            if (selector[i]) {
                candidateSet.emplace_back(*it);
            }
        }

        const auto candidateView =
            std::ranges::subrange(candidateSet.begin(), candidateSet.end()) |
            std::views::transform([](const auto &pair) { return pair.first; });

        if (!host.accommodatesGuests(candidateView.begin(), candidateView.end())) {
            continue;
        }

        const double rewardSum =
            std::accumulate(candidateSet.begin(), candidateSet.end(), 0.,
                            [](const double acc, const auto &guest) { return acc + guest.second; });

        const double subsetValue =
            rewardSum / static_cast<double>(1 + host.countPagesWithGuests(candidateView.begin(),
                                                                          candidateView.end()));

        if (subsetValue > bestSubsetValue) {
            bestSubset = std::move(candidateSet);
            bestSubsetValue = subsetValue;
        }
    } while (std::next_permutation(selector.begin(), selector.end()));

    return bestSubset;
}

/**
 * Places guests on a single host by always picking the next most valuable set
 * based on the reward and page sharing. See Li, et al. (2009) and Rampersaud &
 * Grosu (2014), who proposed a similar algorithm with initialSubsetSize = 1.
 *
 * @param instance the instance to maximise
 * @param profits the profit acquired by packing each guest
 * @param initialSubsetSize the initial subset size to try.
 *
 * O(max(comb(G, 0..initialSubsetSize)) * G^2 * P)
 *
 * Defaults to 1.
 * @return a host with the most valuable guests placed
 */
Host maximiseOneHostBySubsetEfficiency(
    const GeneralInstance &instance,
    const std::unordered_map<std::shared_ptr<const Guest>, int> &profits,
    int initialSubsetSize = 1);

/**
 * Maximises the number of guests placed on a single host on the Cluster Tree
 * model. See Sinderal, et al. (2011).
 *
 * O((C * 2^(2N) * profit_upper_bound^2 + P) * G), where G is the number of guests, C of
 * clusters, N the maximum number of nodes in a cluster and P of pages on a guest
 *
 * @param instance the instance to maximise
 * @param profits the profit acquired by packing each guest
 * @return the maximised host
 */
Host maximiseOneHostByClusterTree(
    const ClusterTreeInstance &instance,
    const std::unordered_map<std::shared_ptr<const Guest>, int> &profits);

/**
 * Maximises the number of guests placed on `allowedHostCount` hosts by using a
 * single-host maximiser. See Fleischer, et al. (2006).
 *
 * O(T_maximiser * allowedHostCount^2 * oneHostApproxRatio * log(1/epsilon) * G * P)
 *
 * @param instance the instance to maximise
 * @param allowedHostCount the number of hosts to use
 * @param oneHostMaximiser the single-host maximiser to use
 * @param oneHostApproxRatio the approximation ratio of the single-host
 * maximiser
 * @param epsilon the approximation factor from Fleischer, et al. (2006). The
 * resulting approximation factor is oneHostApproxRatio/(oneHostApproxRatio + 1) - epsilon.
 * @return a packing with at most `allowedHostCount` hosts
 */
template <typename InstanceType>
    requires Instance<InstanceType>
Packing maximiseByLocalSearch(
    const InstanceType &instance, const size_t allowedHostCount,
    const std::function<Host(const InstanceType &,
                             const std::unordered_map<std::shared_ptr<const Guest>, int> &)>
        &oneHostMaximiser,
    const double oneHostApproxRatio, const double epsilon)
{
    const size_t guestCount = instance.getGuests().size();

    std::vector<std::shared_ptr<Host>> hosts;
    std::unordered_map<std::shared_ptr<const Guest>, std::shared_ptr<Host>> guestHosts;

    hosts.reserve(allowedHostCount);

    for (size_t i = 0; i < allowedHostCount; ++i) {
        const auto host = std::make_shared<Host>(instance.getCapacity());

        for (size_t j = 0; j < guestCount; ++j) {
            const auto guest = instance.getGuests()[j];

            if (!guestHosts.contains(guest) && host->accommodatesGuest(*guest)) {
                host->addGuest(guest);
                guestHosts.emplace(guest, host);
            }
        }

        hosts.push_back(host);
    }

    // Fleischer, et al. iteration count for achieving
    // (oneHostApproxRatio/(oneHostApproxRatio + 1) + epsilon) approximation
    const size_t iterations = std::abs(static_cast<int>(std::ceil(
        static_cast<double>(allowedHostCount) * std::log(1.0 / epsilon) / oneHostApproxRatio)));

    for (size_t iteration = 0; iteration < iterations; ++iteration) {
        int maxImprovement = 0;
        size_t mostImprovableIndex = 0;
        std::shared_ptr<Host> mostImprovingCandidate;

        for (size_t i = 0; i < hosts.size(); ++i) {
            std::unordered_map<std::shared_ptr<const Guest>, int> profits;

            for (const auto &guest : instance.getGuests()) {
                int value = 1;
                if (guestHosts.contains(guest) && guestHosts.at(guest) != hosts[i]) {
                    value = 0;
                }
                profits[guest] = value;
            }

            Host candidate = oneHostMaximiser(instance, profits);
            const int improvement = static_cast<int>(candidate.getGuests().size()) -
                                    static_cast<int>(hosts[i]->getGuests().size());

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

            for (const auto &guest : hosts[mostImprovableIndex]->getGuests()) {
                if (host->hasGuest(guest)) {
                    host->removeGuest(guest);
                    guestHosts[guest] = host;
                }
            }
        }
    }

    size_t count = 0;
    for (const auto &host : hosts) {
        count += host->getGuestCount();
    }

    // Clean up as there is no guarantee that we will utilise all bins
    std::erase_if(hosts,
                  [](const std::shared_ptr<Host> &host) { return host->getGuests().empty(); });
    return Packing(hosts);
}

}  // namespace vmp

#endif  // VMP_MAXIMISERS_H

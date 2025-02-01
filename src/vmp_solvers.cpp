#include <vmp_solvers.h>

#include <vmp_maximisers.h>
#include <vmp_solverutils.h>
#include <vmp_treeinstance.h>

#include <cassert>
#include <iostream>
#include <numeric>
#include <ostream>

namespace vmp
{
/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by Next Fit, modifying a
 * partial hosts vector
 *
 * O(G * P), where G is the guest count and P the maximum amount of pages on any one guest;
 * as we go over every guest and try to pack it into the newest host
 * and to check a guest can be accommodated on a host, we iterate over the guest's pages and check
 * containment in and add to host in O(1)
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<const Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<const Guest> GuestIt>
static void proceedByNextFit(size_t capacity, GuestIt guestsBegin, GuestIt guestsEnd,
                             std::vector<std::shared_ptr<Host>> &hosts)
{
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;
        if (hosts.empty() || !hosts.back()->accommodatesGuest(*guest)) {
            hosts.push_back(std::make_shared<Host>(capacity));
        }
        hosts.back()->addGuest(guest);
    }
}

Packing solveByNextFit(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedByNextFit(instance.getCapacity(), instance.getGuests().begin(),
                     instance.getGuests().end(), hosts);

    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by First Fit, modifying a
 * partial hosts vector
 *
 * O(G^2 * P), where G is the guest count and P the maximum amount of pages on any one guest;
 * as we go over every host for every guest
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<const Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<const Guest> GuestIt>
static void proceedByFirstFit(size_t capacity, GuestIt guestsBegin, GuestIt guestsEnd,
                              std::vector<std::shared_ptr<Host>> &hosts)
{
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;

        auto hostIter = std::ranges::find_if(
            hosts, [&](const auto &host) { return host->accommodatesGuest(*guest); });
        if (hostIter == hosts.end()) {
            hosts.push_back(std::make_shared<Host>(capacity));
            hostIter = hosts.end() - 1;
        }

        (*hostIter)->addGuest(guest);
    }
}

Packing solveByFirstFit(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedByFirstFit(instance.getCapacity(), instance.getGuests().begin(),
                      instance.getGuests().end(), hosts);

    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by "Best Fusion" of Grange, et
 * al. (2021), modifying a partial hosts vector
 *
 * O(G^3 * P)
 *
 * as we do
 * O(P) + O(G^2 * 2P) + partition_time + O(G^2 * partitions * (P + P*G/partitions))
 * where
 *  partition_time = O(G^2 * P)
 *  partitions = O(1) or O(G) (including in the component case)
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<const Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<const Guest> GuestIt>
static void proceedByBestFusion(size_t capacity, GuestIt guestsBegin, GuestIt guestsEnd,
                                std::vector<std::shared_ptr<Host>> &hosts)
{
    const auto frequencies = calculatePageFrequencies(guestsBegin, guestsEnd);

    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;

        double bestRelSize = guest->getUniquePageCount();
        std::shared_ptr<Host> bestHost = nullptr;

        for (const auto &host : hosts) {
            if (!host->accommodatesGuest(*guest)) {
                continue;
            }

            const double candidateRelSize = calculateRelSize(*guest, frequencies);
            if (candidateRelSize < bestRelSize) {
                bestHost = host;
                bestRelSize = candidateRelSize;
            }
        }

        if (!bestHost) {
            hosts.emplace_back(std::make_shared<Host>(capacity));
            bestHost = hosts.back();
        }
        bestHost->addGuest(*guestsBegin);
    }

    using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
    decantGuests<SetGuestIt>(hosts, makeOneGuestPartition<SetGuestIt>);
    decantGuests<SetGuestIt>(hosts, makeShareGraphComponentGuestPartitions<SetGuestIt>);
    decantGuests<SetGuestIt>(hosts, makeIndividualGuestPartitions<SetGuestIt>);
}

Packing solveByBestFusion(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    const auto &guests = instance.getGuests();
    proceedByBestFusion(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by "Overload-and-Remove" of
 * Grange, et al. (2021), modifying a partial hosts vector
 *
 * O(G^3 * P)
 *
 * as we do O(P) + O(G * (G*P + G*(G*P + P))
 *
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<const Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<const Guest> GuestIt>
static void proceedByOverloadAndRemove(size_t capacity, GuestIt guestsBegin, GuestIt guestsEnd,
                                       std::vector<std::shared_ptr<Host>> &hosts)
{
    std::deque unplaced(guestsBegin, guestsEnd);
    std::unordered_map<std::shared_ptr<const Guest>, std::unordered_set<std::shared_ptr<Host>>>
        attemptedPlacements;
    const auto frequencies = calculatePageFrequencies(guestsBegin, guestsEnd);

    while (!unplaced.empty()) {
        // Select best container based on relative size
        const auto guest = unplaced.front();
        unplaced.pop_front();

        std::shared_ptr<Host> bestHost = nullptr;
        double bestRelSize = std::numeric_limits<double>::max();

        for (const auto &host : hosts) {
            if (attemptedPlacements[guest].contains(host)) {
                continue;
            }
            const auto candidateRelSize = calculateRelSize(*guest, frequencies);
            if (candidateRelSize < bestRelSize) {
                bestHost = host;
                bestRelSize = candidateRelSize;
            }
        }

        if (!bestHost) {
            hosts.emplace_back(std::make_shared<Host>(capacity));
            bestHost = hosts.back();
        }

        bestHost->addGuest(guest);
        attemptedPlacements[guest].insert(bestHost);

        // Remove worst guest in container
        while (bestHost->isOverfull()) {
            const auto worstGuest =
                *std::ranges::min_element(bestHost->getGuests(), {}, [&](const auto &candidate) {
                    return calculateSizeRelRatio(*candidate, frequencies);
                });

            unplaced.push_back(worstGuest);
            bestHost->removeGuest(worstGuest);
        }
    }

    // Clear overfull containers and enqueue their guests
    for (const auto &host : hosts) {
        if (!host->isOverfull()) {
            continue;
        }
        for (const auto &guest : host->getGuests()) {
            unplaced.push_back(guest);
        }
        host->clearGuests();
    }

    proceedByFirstFit(capacity, unplaced.begin(), unplaced.end(), hosts);

    using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
    decantGuests<SetGuestIt>(hosts, makeOneGuestPartition<SetGuestIt>);
    decantGuests<SetGuestIt>(hosts, makeShareGraphComponentGuestPartitions<SetGuestIt>);
    decantGuests<SetGuestIt>(hosts, makeIndividualGuestPartitions<SetGuestIt>);
}

Packing solveByOverloadAndRemove(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedByOverloadAndRemove(instance.getCapacity(), instance.getGuests().begin(),
                               instance.getGuests().end(), hosts);

    return Packing(hosts);
}

Packing solveByLocalityScore(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    std::unordered_set unplaced(instance.getGuests().begin(), instance.getGuests().end());

    while (!unplaced.empty()) {
        std::shared_ptr<const Guest> largestGuest;
        std::shared_ptr<const Guest> bestGuest;
        std::shared_ptr<Host> bestHost;
        double bestScore = std::numeric_limits<double>::min();

        for (const auto &guest : unplaced) {
            if (!largestGuest || guest->getUniquePageCount() > largestGuest->getUniquePageCount()) {
                largestGuest = guest;
            }

            for (const auto &host : hosts) {
                if (!host->accommodatesGuest(*guest)) {
                    continue;
                }
                const double candidateScore = calculateLocalityScore(*guest, host, hosts);
                if (candidateScore > bestScore) {
                    bestGuest = guest;
                    bestHost = host;
                    bestScore = candidateScore;
                }
            }
        }

        if (!bestGuest) {
            bestHost = std::make_shared<Host>(instance.getCapacity());
            bestGuest = largestGuest;
            hosts.push_back(bestHost);
        }
        bestHost->addGuest(bestGuest);
        unplaced.erase(bestGuest);
    }

    return Packing(hosts);
}

/*
 * O((G^2 * P) + N^2), where N is the number of nodes in the tree (bounded above by G + total number
 * of unique pages in the instance)
 *
 * as we have
 * O(G^2 * P) for the sum of all the first-fits (each guest is packed exactly once)
 * plus, in the worst case, each subtree we remove has size N / 2, so the loop runs O(N / 2) times
 * and each subtree removal costs O(N / 2) and iterating over the subtree's nodes is O(2 * N/2),
 * as each node is considered twice, once as a child, once as a parent
 */
Packing solveBySimpleTree(const TreeInstance &instance)
{
    TreeInstance instanceCopy = instance;

    std::vector<std::shared_ptr<Host>> hosts;

    while (true) {
        const auto lowerBounds = calculateAllSubtreeLowerBounds(instanceCopy);

        if (lowerBounds.at(TreeInstance::getRootNode()).count == 1) {
            const auto &guests = instanceCopy.getGuests();
            if (guests.empty()) {
                break;
            }

            Host host(instanceCopy.getCapacity());
            host.addGuests(guests.begin(), guests.end());

            hosts.push_back(std::make_shared<Host>(host));
            break;
        }

        size_t minNode = std::numeric_limits<size_t>::max();
        size_t minNodeCount = std::numeric_limits<size_t>::max();

        for (const auto &[node, bounds] : lowerBounds) {
            if (bounds.count <= 1) {
                continue;
            }

            const auto &children = instanceCopy.getNodeChildren(node);
            if (!std::ranges::all_of(children, [&](const size_t child) {
                    return lowerBounds.at(child).count == 1;
                })) {
                continue;
            }

            if (minNode == std::numeric_limits<size_t>::max() || bounds.count < minNodeCount) {
                minNode = node;
                minNodeCount = bounds.count;
            }
        }

        assert(minNode != std::numeric_limits<size_t>::max());

        const auto &guestsToPack = instanceCopy.getSubtreeGuests(minNode);
        proceedByFirstFit(instanceCopy.getCapacity(), guestsToPack.begin(), guestsToPack.end(),
                          hosts);

        if (minNode == TreeInstance::getRootNode()) {
            break;
        }

        instanceCopy.removeSubtree(minNode);
    }

    return Packing(hosts);
}

Packing solveBySubsetEfficiency(const GeneralInstance &instance, int initialSubsetSize)
{
    const double epsilon = 0.0001;    // TODO
    const double oneHostApprox = 25;  // TODO

    auto oneHostMaximiser =
        [&](const GeneralInstance &inst,
            const std::unordered_map<std::shared_ptr<const Guest>, int> &profits) {
            return maximiseOneHostBySubsetEfficiency(inst, profits, initialSubsetSize);
        };

    auto nHostMaximiser = [&](const GeneralInstance &inst, const size_t maxHosts) {
        return maximiseByLocalSearch<GeneralInstance>(inst, maxHosts, oneHostMaximiser,
                                                      oneHostApprox, epsilon);
    };

    return solveByMaximiser<GeneralInstance>(instance, nHostMaximiser);
}

Packing solveByClusterTree(const ClusterTreeInstance &instance)
{
    const double epsilon = 0.0001;    // TODO
    const double oneHostApprox = 25;  // TODO

    auto oneHostMaximiser =
        [&](const ClusterTreeInstance &inst,
            const std::unordered_map<std::shared_ptr<const Guest>, int> &profits) {
            return maximiseOneHostByClusterTree(inst, profits);
        };

    auto nHostMaximiser = [&](const ClusterTreeInstance &inst, const size_t maxHosts) {
        return maximiseByLocalSearch<ClusterTreeInstance>(inst, maxHosts, oneHostMaximiser,
                                                          oneHostApprox, epsilon);
    };

    return solveByMaximiser<ClusterTreeInstance>(instance, nHostMaximiser);
}

}  // namespace vmp

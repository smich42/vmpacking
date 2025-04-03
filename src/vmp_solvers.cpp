#include <vmp_solvers.h>

#include <vmp_maximisers.h>
#include <vmp_solverutils.h>
#include <vmp_treeinstance.h>

#include <iostream>
#include <numeric>
#include <ostream>
#include <random>

namespace vmp
{

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by Next Fit, modifying a
 * partial hosts vector
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

Packing solveByNextFit(const GeneralInstance &instance, const bool decant)
{
    std::vector<std::shared_ptr<Host>> hosts;

    auto guests = instance.getGuests();
    proceedByNextFit(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }
    return Packing(hosts);
}

// TODO consolidate these
Packing solveByNextFit(const TreeInstance &instance, const bool decant)
{
    std::vector<std::shared_ptr<Host>> hosts;

    const auto &guests = instance.getGuests();
    proceedByNextFit(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }

    return Packing(hosts);
}

Packing solveByNextFit(const ClusterTreeInstance &instance, const bool decant)
{
    std::vector<std::shared_ptr<Host>> hosts;

    const auto &guests = instance.getGuests();
    proceedByNextFit(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }
    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by First Fit, modifying a
 * partial hosts vector
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
    auto guests = instance.getGuests();

    proceedByFirstFit(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    return Packing(hosts);
}

// TODO consolidate these
Packing solveByFirstFit(const TreeInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;

    const auto &guests = instance.getGuests();
    proceedByFirstFit(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    return Packing(hosts);
}

Packing solveByFirstFit(const ClusterTreeInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;

    const auto &guests = instance.getGuests();
    proceedByFirstFit(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by "Best Fusion" of Grange, et
 * al. (2021), modifying a partial hosts vector
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<const Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<const Guest> GuestIt>
static void proceedByEfficiency(size_t capacity, GuestIt guestsBegin, GuestIt guestsEnd,
                                std::vector<std::shared_ptr<Host>> &hosts)
{
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;

        double bestRelSize = guest->getUniquePageCount();
        std::shared_ptr<Host> bestHost = nullptr;

        for (const auto &host : hosts) {
            if (!host->accommodatesGuest(*guest)) {
                continue;
            }

            const double candidateRelSize = calculateRelSize(*guest, host->getPageFrequencies());
            if (candidateRelSize <= bestRelSize) {
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
}

Packing solveByEfficiency(const GeneralInstance &instance, const bool decant)
{
    std::vector<std::shared_ptr<Host>> hosts;
    const auto &guests = instance.getGuests();
    proceedByEfficiency(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }
    return Packing(hosts);
}

Packing solveByEfficiency(const ClusterTreeInstance &instance, const bool decant)
{
    std::vector<std::shared_ptr<Host>> hosts;
    const auto &guests = instance.getGuests();
    proceedByEfficiency(instance.getCapacity(), guests.begin(), guests.end(), hosts);

    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }
    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by "Overload-and-Remove" of
 * Grange, et al. (2021), modifying a partial hosts vector
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<const Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 * @param decant whether to apply the decanting post-treatment
 */
template <SharedPtrIterator<const Guest> GuestIt>
static void proceedByOverloadAndRemove(size_t capacity, GuestIt guestsBegin, GuestIt guestsEnd,
                                       std::vector<std::shared_ptr<Host>> &hosts, const bool decant)
{
    std::deque unplaced(guestsBegin, guestsEnd);
    std::unordered_map<std::shared_ptr<const Guest>, std::unordered_set<std::shared_ptr<Host>>>
        attemptedPlacements;
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
            const auto candidateRelSize = calculateRelSize(*guest, host->getPageFrequencies());
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
                    return calculateSizeRelRatio(*candidate, bestHost->getPageFrequencies());
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

    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }
}

Packing solveByOverloadAndRemove(const GeneralInstance &instance, const bool decant)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedByOverloadAndRemove(instance.getCapacity(), instance.getGuests().begin(),
                               instance.getGuests().end(), hosts, decant);

    return Packing(hosts);
}

Packing solveByOverloadAndRemove(const ClusterTreeInstance &instance, const bool decant)
{
    std::vector<std::shared_ptr<Host>> hosts;
    const auto &guests = instance.getGuests();
    proceedByOverloadAndRemove(instance.getCapacity(), guests.begin(), guests.end(), hosts, decant);

    return Packing(hosts);
}

Packing solveByOpportunityAwareEfficiency(const GeneralInstance &instance, const bool decant)
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
                const double candidateScore =
                    calculateOpportunityAwareEfficiency(*guest, host, hosts);
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

    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }
    return Packing(hosts);
}

// TODO consolidate this
Packing solveByOpportunityAwareEfficiency(const ClusterTreeInstance &instance, const bool decant)
{
    std::vector<std::shared_ptr<Host>> hosts;
    const auto &guests = instance.getGuests();
    std::unordered_set unplaced(guests.begin(), guests.end());

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
                const double candidateScore =
                    calculateOpportunityAwareEfficiency(*guest, host, hosts);
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

    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }
    return Packing(hosts);
}

Packing solveByLocalSubsetEfficiency(const GeneralInstance &instance, const int initialSubsetSize,
                                     const bool decant)
{
    auto oneHostMaximiser =
        [&](const GeneralInstance &inst,
            const std::unordered_map<std::shared_ptr<const Guest>, int> &profits) {
            return maximiseOneHostBySubsetEfficiency(inst, profits, initialSubsetSize);
        };

    auto nHostMaximiser = [&](const GeneralInstance &inst, const size_t maxHosts) {
        return maximiseByLocalSearch<GeneralInstance>(inst, maxHosts, oneHostMaximiser);
    };

    return solveByMaximiser<GeneralInstance>(instance, nHostMaximiser, true, decant);
}

Packing solveByLocalClusterTree(const ClusterTreeInstance &instance, const bool decant)
{
    auto oneHostMaximiser =
        [&](const ClusterTreeInstance &inst,
            const std::unordered_map<std::shared_ptr<const Guest>, int> &profits) {
            return maximiseOneHostByClusterTree(inst, profits);
        };

    auto nHostMaximiser = [&](const ClusterTreeInstance &inst, const size_t maxHosts) {
        return maximiseByLocalSearch<ClusterTreeInstance>(inst, maxHosts, oneHostMaximiser);
    };

    return solveByMaximiser<ClusterTreeInstance>(instance, nHostMaximiser, true, decant);
}

}  // namespace vmp

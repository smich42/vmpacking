#include <vmp_solvers.h>

#include <numeric>
#include <ostream>
#include <vmp_solverutils.h>

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
static void proceedByNextFit(size_t capacity, GuestIt guestsBegin,
                             GuestIt guestsEnd,
                             std::vector<std::shared_ptr<Host>> &hosts)
{
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;
        if (hosts.empty() || !hosts.back()->accommodatesGuest(*guest)) {
            hosts.emplace_back(std::make_shared<Host>(capacity));
        }
        hosts.back()->addGuest(guest);
    }
}

Packing solveByNextFit(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedByNextFit(instance.capacity, instance.guests.begin(),
                     instance.guests.end(), hosts);

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
static void proceedByFirstFit(size_t capacity, GuestIt guestsBegin,
                              GuestIt guestsEnd,
                              std::vector<std::shared_ptr<Host>> &hosts)
{
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;

        auto hostIter = std::ranges::find_if(hosts, [&](const auto &host) {
            return host->accommodatesGuest(*guest);
        });
        if (hostIter == hosts.end()) {
            hosts.emplace_back(std::make_shared<Host>(capacity));
            hostIter = hosts.end() - 1;
        }

        (*hostIter)->addGuest(guest);
    }
}

Packing solveByFirstFit(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedByFirstFit(instance.capacity, instance.guests.begin(),
                      instance.guests.end(), hosts);

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
static void proceedByBestFusion(size_t capacity, GuestIt guestsBegin,
                                GuestIt guestsEnd,
                                std::vector<std::shared_ptr<Host>> &hosts)
{
    const auto frequencies = calculatePageFrequencies(guestsBegin, guestsEnd);

    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;

        double bestRelSize = guest->pageCount();
        std::shared_ptr<Host> bestHost = nullptr;

        for (const auto &host : hosts) {
            if (!host->accommodatesGuest(*guest)) {
                continue;
            }

            const double candidateRelSize =
                calculateRelSize(*guest, frequencies);
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

    // TODO ordered set only provides quasi-deterministic behaviour; depends on
    // raw pointer addresses
    using SetGuestIt = std::set<std::shared_ptr<const Guest>>::iterator;
    decantGuests<SetGuestIt>(hosts, makeOneGuestPartition<SetGuestIt>);
    decantGuests<SetGuestIt>(
        hosts, makeShareGraphComponentGuestPartitions<SetGuestIt>);
    decantGuests<SetGuestIt>(hosts, makeIndividualGuestPartitions<SetGuestIt>);
}

Packing solveByBestFusion(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedByBestFusion(instance.capacity, instance.guests.begin(),
                        instance.guests.end(), hosts);

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
 */
template <SharedPtrIterator<const Guest> GuestIt>
static void
proceedByOverloadAndRemove(size_t capacity, GuestIt guestsBegin,
                           GuestIt guestsEnd,
                           std::vector<std::shared_ptr<Host>> &hosts)
{
    std::deque unplaced(guestsBegin, guestsEnd);
    std::map<std::shared_ptr<const Guest>, std::set<std::shared_ptr<Host>>>
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
            const auto candidateRelSize =
                calculateRelSize(*guest, frequencies);
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
            const auto worstGuest = *std::ranges::min_element(
                bestHost->guests, {}, [&](const auto &candidate) {
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
        for (const auto &guest : host->guests) {
            unplaced.push_back(guest);
        }
        host->clearGuests();
    }

    proceedByFirstFit(capacity, unplaced.begin(), unplaced.end(), hosts);

    using SetGuestIt = std::set<std::shared_ptr<const Guest>>::iterator;
    decantGuests<SetGuestIt>(hosts, makeOneGuestPartition<SetGuestIt>);
    decantGuests<SetGuestIt>(
        hosts, makeShareGraphComponentGuestPartitions<SetGuestIt>);
    decantGuests<SetGuestIt>(hosts, makeIndividualGuestPartitions<SetGuestIt>);
}

Packing solveByOverloadAndRemove(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedByOverloadAndRemove(instance.capacity, instance.guests.begin(),
                               instance.guests.end(), hosts);

    return Packing(hosts);
}

Packing solveByLocalityScore(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    std::set unplaced(instance.guests.begin(), instance.guests.end());

    while (!unplaced.empty()) {
        std::shared_ptr<const Guest> largestGuest;
        std::shared_ptr<const Guest> bestGuest;
        std::shared_ptr<Host> bestHost;
        double bestScore = std::numeric_limits<double>::min();

        for (const auto &guest : unplaced) {
            if (!largestGuest ||
                guest->pageCount() > largestGuest->pageCount()) {
                largestGuest = guest;
            }

            for (const auto &host : hosts) {
                if (!host->accommodatesGuest(*guest)) {
                    continue;
                }
                const double candidateScore =
                    calculateLocalityScore(*guest, host, hosts);
                if (candidateScore > bestScore) {
                    bestGuest = guest;
                    bestHost = host;
                    bestScore = candidateScore;
                }
            }
        }

        if (!bestGuest) {
            bestHost = std::make_shared<Host>(instance.capacity);
            bestGuest = largestGuest;
            hosts.push_back(bestHost);
        }
        bestHost->addGuest(bestGuest);
        unplaced.erase(bestGuest);
    }

    return Packing(hosts);
}

Packing solveByMaximiser(const GeneralInstance &instance,
                         Packing (*maximiser)(const GeneralInstance &instance,
                                              size_t maxHosts))
{
    size_t minHosts = 0;
    size_t maxHosts = instance.guestCount() + 1;

    std::shared_ptr<Packing> bestPacking;

    while (minHosts <= maxHosts) {
        const size_t allowedHostCount = (minHosts + maxHosts) / 2;
        Packing candidate = maximiser(instance, allowedHostCount);

        if (candidate.countGuests() == instance.guestCount()) {
            bestPacking = std::make_shared<Packing>(std::move(candidate));
            maxHosts = allowedHostCount - 1;
        }
        else {  // Not all guests could be packed
            minHosts = allowedHostCount + 1;
        }
    }

    if (bestPacking == nullptr) {
        throw std::runtime_error(
            "No valid packing found; is a guest larger than the capacity?");
    }
    return *bestPacking;
}

}  // namespace vmp

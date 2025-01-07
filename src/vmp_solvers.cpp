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
 * @tparam GuestIt any iterator type over `std::shared_ptr<Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<Guest> GuestIt>
static void proceedNextFit(size_t capacity, GuestIt guestsBegin,
                           const GuestIt guestsEnd,
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

/**
 * Solves an instance of VM-PACK by Next Fit
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByNextFit(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedNextFit(instance.capacity, instance.guests.begin(),
                   instance.guests.end(), hosts);

    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by First Fit, modifying a
 * partial hosts vector
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<Guest> GuestIt>
static void proceedFirstFit(size_t capacity, GuestIt guestsBegin,
                            const GuestIt guestsEnd,
                            std::vector<std::shared_ptr<Host>> &hosts)
{
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;

        auto hostIter = std::ranges::find_if(hosts, [&](const auto host) {
            return host->accommodatesGuest(*guest);
        });
        if (hostIter == hosts.end()) {
            hosts.emplace_back(std::make_shared<Host>(capacity));
            hostIter = hosts.end() - 1;
        }

        (*hostIter)->addGuest(guest);
    }
}

/**
 * Solves an instance of VM-PACK by First Fit
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByFirstFit(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedFirstFit(instance.capacity, instance.guests.begin(),
                    instance.guests.end(), hosts);

    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by "Best Fusion" of Grange, et
 * al. (2021), modifying a partial hosts vector
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<Guest> GuestIt>
static void proceedBestFusion(size_t capacity, GuestIt guestsBegin,
                              const GuestIt guestsEnd,
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

    // Use ordered set for deterministic behaviour
    using SetGuestIt = std::set<std::shared_ptr<Guest>>::iterator;
    decantGuests<SetGuestIt>(hosts, makeOneGuestPartition<SetGuestIt>);
    decantGuests<SetGuestIt>(
        hosts, makeShareGraphComponentGuestPartitions<SetGuestIt>);
    decantGuests<SetGuestIt>(hosts, makeIndividualGuestPartitions<SetGuestIt>);
}

/**
 * Solves an instance of VM-PACK by "Best Fusion" of Grange, et
 * al. (2021)
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByBestFusion(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedBestFusion(instance.capacity, instance.guests.begin(),
                      instance.guests.end(), hosts);

    return Packing(hosts);
}

/**
 * Packs `[guestsBegin, guestsEnd)` sequentially by "Overload-and-Remove" of
 * Grange, et al. (2021), modifying a partial hosts vector
 *
 * @tparam GuestIt any iterator type over `std::shared_ptr<Guest>`
 * @param capacity the fixed bin capacity
 * @param guestsBegin the start of guest range
 * @param guestsEnd the end of guest range
 * @param hosts the partial hosts vector to use
 */
template <SharedPtrIterator<Guest> GuestIt>
static void proceedOverloadAndRemove(size_t capacity, GuestIt guestsBegin,
                                     const GuestIt guestsEnd,
                                     std::vector<std::shared_ptr<Host>> &hosts)
{
    std::deque<std::shared_ptr<Guest>> unplacedGuests;
    std::map<std::shared_ptr<Guest>, std::set<std::shared_ptr<Host>>>
        attemptedPlacements;
    const auto frequencies = calculatePageFrequencies(guestsBegin, guestsEnd);

    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        unplacedGuests.emplace_back(*guestsBegin);
    }

    while (!unplacedGuests.empty()) {
        // Select best container based on relative size
        const auto guest = unplacedGuests.front();
        unplacedGuests.pop_front();

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

            unplacedGuests.push_back(worstGuest);
            bestHost->removeGuest(worstGuest);
        }
    }

    // Clear overfull containers and enqueue their guests
    for (const auto &host : hosts) {
        if (!host->isOverfull()) {
            continue;
        }
        for (const auto &guest : host->guests) {
            unplacedGuests.push_back(guest);
        }
        host->clearGuests();
    }

    proceedFirstFit(capacity, unplacedGuests.begin(), unplacedGuests.end(),
                    hosts);

    using SetGuestIt = std::set<std::shared_ptr<Guest>>::iterator;
    decantGuests<SetGuestIt>(hosts, makeOneGuestPartition<SetGuestIt>);
    decantGuests<SetGuestIt>(
        hosts, makeShareGraphComponentGuestPartitions<SetGuestIt>);
    decantGuests<SetGuestIt>(hosts, makeIndividualGuestPartitions<SetGuestIt>);
}

/**
 * Solves an instance of VM-PACK by "Overload-and-Remove" of Grange, et
 * al. (2021)
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByOverloadAndRemove(const GeneralInstance &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedOverloadAndRemove(instance.capacity, instance.guests.begin(),
                             instance.guests.end(), hosts);

    return Packing(hosts);
}

Packing
solveByMaximiser(const GeneralInstance &instance,
                 const std::function<Packing(const GeneralInstance &instance,
                                             size_t maxHosts)> &maximiser)
{
    size_t maxHostsLb = 0;
    size_t maxHostsUb = instance.guests.size();
    Packing bestPacking({});

    while (maxHostsLb <= maxHostsUb) {
        const size_t maxHostsCandidate = (maxHostsLb + maxHostsUb) / 2;
        auto packingCandidate = maximiser(instance, maxHostsCandidate);

        if (packingCandidate.countGuests() == instance.guests.size()) {
            maxHostsUb = maxHostsCandidate;
            bestPacking = std::move(packingCandidate);
        }
        else {  // Not all guests could be packed
            maxHostsLb = maxHostsCandidate + 1;
        }
    }

    return bestPacking;
}

}  // namespace vmp

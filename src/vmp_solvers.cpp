#include <vmp_solvers.h>

#include <numeric>
#include <ostream>
#include <vmp_solverutils.h>

namespace vmp
{
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

Packing solveNextFit(const std::shared_ptr<Instance> &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedNextFit(instance->capacity, instance->guests.begin(),
                   instance->guests.end(), hosts);

    return Packing(instance, hosts);
}

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
            hostIter = std::prev(hosts.end());
        }

        (*hostIter)->addGuest(guest);
    }
}

Packing solveFirstFit(const std::shared_ptr<Instance> &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedFirstFit(instance->capacity, instance->guests.begin(),
                    instance->guests.end(), hosts);

    return Packing(instance, hosts);
}

template <SharedPtrIterator<Guest> GuestIt>
static void proceedBestFusion(size_t capacity, GuestIt guestsBegin,
                              const GuestIt guestsEnd,
                              std::vector<std::shared_ptr<Host>> &hosts)
{
    const std::unordered_map<int, int> frequencies =
        calculatePageFrequencies(guestsBegin, guestsEnd);

    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        const auto &guest = *guestsBegin;

        double bestRelSize = guest->pageCount();
        std::shared_ptr<Host> bestHost = nullptr;

        for (const auto &host : hosts) {
            if (!host->accommodatesGuest(*guest)) {
                continue;
            }

            if (const double candidateRelSize =
                    relativeSize(guest, frequencies);
                candidateRelSize < bestRelSize) {
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

Packing solveBestFusion(const std::shared_ptr<Instance> &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedBestFusion(instance->capacity, instance->guests.begin(),
                      instance->guests.end(), hosts);

    return Packing(instance, hosts);
}

template <SharedPtrIterator<Guest> GuestIt>
static void proceedOverloadAndRemove(size_t capacity, GuestIt guestsBegin,
                                     const GuestIt guestsEnd,
                                     std::vector<std::shared_ptr<Host>> &hosts)
{
    std::deque<std::shared_ptr<Guest>> unplacedGuests;
    std::map<std::shared_ptr<Guest>, std::set<std::shared_ptr<Host>>>
        attemptedPlacements;
    const std::unordered_map<int, int> frequencies =
        calculatePageFrequencies(guestsBegin, guestsEnd);

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
            if (const auto candidateRelSize = relativeSize(guest, frequencies);
                candidateRelSize < bestRelSize) {
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
            // const bool allSameEfficiency =
            //     std::ranges::adjacent_find(
            //         bestHost->guests,
            //         [&frequencies](const auto &guestA, const auto &guestB) {
            //             constexpr double relEps = 1e-4;
            //             const double valA =
            //                 sizeOverRelativeSize(guestA, frequencies);
            //             const double valB =
            //                 sizeOverRelativeSize(guestB, frequencies);
            //             return std::abs(valA - valB) >
            //                    relEps *
            //                        std::max(std::abs(valA), std::abs(valB));
            //         }) == bestHost->guests.end();
            //
            // if (allSameEfficiency) {
            //     break;
            // }
            const auto worstGuest = *std::ranges::min_element(
                bestHost->guests, {}, [&](const auto &candidate) {
                    return sizeOverRelativeSize(candidate, frequencies);
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

    // Remove any redundant containers
    std::erase_if(hosts,
                  [](const auto &host) { return host->guestCount() == 0; });
}

Packing solveOverloadAndRemove(const std::shared_ptr<Instance> &instance)
{
    std::vector<std::shared_ptr<Host>> hosts;
    proceedOverloadAndRemove(instance->capacity, instance->guests.begin(),
                             instance->guests.end(), hosts);

    return Packing(instance, hosts);
}

}  // namespace vmp

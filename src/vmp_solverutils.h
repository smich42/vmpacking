#ifndef VMP_SOLVERUTILS_H
#define VMP_SOLVERUTILS_H

#include <vmp_solverutils.h>

#include <ranges>
#include <unordered_set>
#include <vmp_guest.h>
#include <vmp_host.h>
#include <vmp_iterators.h>

namespace vmp
{

double calculateRelSize(const std::shared_ptr<Guest> &guest,
                        const std::unordered_map<int, int> &pageFreq);

double calculateSizeRelRatio(const std::shared_ptr<Guest> &guest,
                             const std::unordered_map<int, int> &pageFreq);

template <SharedPtrIterator<Guest> GuestIt>
void decantGuests(
    std::vector<std::shared_ptr<Host>> &hosts,
    const std::function<std::vector<std::vector<std::shared_ptr<Guest>>>(
        GuestIt, GuestIt)> &partitionGuests)
{
    for (auto leftIt = hosts.begin(); leftIt != hosts.end(); ++leftIt) {
        const auto &leftHost = *leftIt;

        for (auto rightIt = leftIt + 1; rightIt != hosts.end(); ++rightIt) {
            const auto &rightHost = *rightIt;
            const auto partitions = partitionGuests(rightHost->guests.begin(),
                                                    rightHost->guests.end());

            for (const auto &partition : partitions) {
                const bool accommodatesAll =
                    std::ranges::all_of(partition, [&](const auto &guest) {
                        return leftHost->accommodatesGuest(*guest);
                    });
                if (!accommodatesAll) {
                    continue;
                }

                for (const auto &guest : partition) {
                    leftHost->addGuest(guest);
                    rightHost->removeGuest(guest);
                }
            }
        }
    }
    std::erase_if(hosts,
                  [](const auto &host) { return host->guestCount() == 0; });
}

template <SharedPtrIterator<Guest> GuestIt>
std::unordered_map<int, int> calculatePageFrequencies(GuestIt guestsBegin,
                                                      GuestIt guestsEnd)
{
    std::unordered_map<int, int> pageFreq;
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        for (const auto &page : (*guestsBegin)->pages) {
            ++pageFreq[page];
        }
    }
    return pageFreq;
}

template <SharedPtrIterator<Guest> GuestIt>
std::vector<std::vector<std::shared_ptr<Guest>>>
makeOneGuestPartition(GuestIt guestsBegin, GuestIt guestsEnd)
{
    // Whole-page decanting
    std::vector<std::shared_ptr<Guest>> allGuests;
    for (auto it = guestsBegin; it != guestsEnd; ++it) {
        allGuests.push_back(*it);
    }
    return { allGuests };
}

template <SharedPtrIterator<Guest> GuestIt>
std::vector<std::vector<std::shared_ptr<Guest>>>
makeIndividualGuestPartitions(GuestIt guestsBegin, GuestIt guestsEnd)
{
    // Per-guest decanting
    std::vector<std::vector<std::shared_ptr<Guest>>> partition;
    for (auto guestIt = guestsBegin; guestIt != guestsEnd; ++guestIt) {
        partition.push_back(std::vector{ *guestIt });
    }
    return partition;
}

inline bool guestsHaveSharedPage(const Guest &guest1, const Guest &guest2)
{
    return std::ranges::any_of(guest1.pages, [&](const auto &page1) {
        return guest2.pages.contains(page1);
    });
}

template <SharedPtrIterator<Guest> GuestIt>
std::vector<std::vector<std::shared_ptr<Guest>>>
makeShareGraphComponentGuestPartitions(GuestIt guestsBegin, GuestIt guestsEnd)
{
    std::unordered_set<std::shared_ptr<Guest>> visited;
    std::vector<std::vector<std::shared_ptr<Guest>>> result;

    std::function<void(GuestIt, std::vector<std::shared_ptr<Guest>> &)> dfs =
        [&](GuestIt cur, std::vector<std::shared_ptr<Guest>> &component) {
            visited.insert(*cur);
            component.push_back(*cur);

            for (auto it = cur; it != guestsEnd; ++it) {
                if (!visited.contains(*it) &&
                    guestsHaveSharedPage(**cur, **it)) {
                    dfs(it, component);
                }
            }
        };

    for (auto it = guestsBegin; it != guestsEnd; ++it) {
        if (visited.contains(*it)) {
            continue;
        }
        std::vector<std::shared_ptr<Guest>> component;
        dfs(it, component);
        result.push_back(std::move(component));
    }

    return result;
}

}  // namespace vmp

#endif  // VMP_SOLVERUTILS_H

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

double calculateRelSize(const Guest &guest, const std::unordered_map<int, int> &pageFreq);

double calculateSizeRelRatio(const Guest &guest, const std::unordered_map<int, int> &pageFreq);

double calculateLocalityScore(const Guest &guest, const std::shared_ptr<Host> &host,
                              const std::vector<std::shared_ptr<Host>> &allHosts);

double countGuestPagesPlaced(const Guest &guest, const std::vector<std::shared_ptr<Host>> &hosts);

template <SharedPtrIterator<const Guest> GuestIt>
void decantGuests(
    std::vector<std::shared_ptr<Host>> &hosts,
    std::vector<std::vector<std::shared_ptr<const Guest>>> (*partitionGuests)(GuestIt, GuestIt))
{
    for (auto leftIt = hosts.begin(); leftIt != hosts.end(); ++leftIt) {
        const auto &leftHost = *leftIt;

        for (auto rightIt = leftIt + 1; rightIt != hosts.end(); ++rightIt) {
            const auto &rightHost = *rightIt;
            const auto partitions =
                partitionGuests(rightHost->guests.begin(), rightHost->guests.end());

            for (const auto &partition : partitions) {
                if (!leftHost->accommodatesGuests(partition.begin(), partition.end())) {
                    continue;
                }

                for (const auto &guest : partition) {
                    leftHost->addGuest(guest);
                    rightHost->removeGuest(guest);
                }
            }
        }
    }
    std::erase_if(hosts, [](const auto &host) { return host->guestCount() == 0; });
}

template <SharedPtrIterator<const Guest> GuestIt>
std::unordered_map<int, int> calculatePageFrequencies(GuestIt guestsBegin, GuestIt guestsEnd)
{
    std::unordered_map<int, int> frequencies;
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        for (const auto &page : (*guestsBegin)->pages) {
            ++frequencies[page];
        }
    }
    return frequencies;
}

template <SharedPtrIterator<const Host> HostIt>
std::unordered_map<int, int> calculatePageFrequencies(HostIt hostsBegin, HostIt hostsEnd)
{
    std::unordered_map<int, int> frequencies;
    for (; hostsBegin != hostsEnd; ++hostsBegin) {
        for (const auto &guest : (*hostsBegin)->guests) {
            for (const auto &page : guest->pages) {
                ++frequencies[page];
            }
        }
    }
    return frequencies;
}

template <SharedPtrIterator<const Guest> GuestIt>
std::vector<std::vector<std::shared_ptr<const Guest>>> makeOneGuestPartition(GuestIt guestsBegin,
                                                                             GuestIt guestsEnd)
{
    // Whole-page decanting
    std::vector<std::shared_ptr<const Guest>> allGuests;
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        allGuests.push_back(*guestsBegin);
    }
    return { allGuests };
}

template <SharedPtrIterator<const Guest> GuestIt>
std::vector<std::vector<std::shared_ptr<const Guest>>>
makeIndividualGuestPartitions(GuestIt guestsBegin, GuestIt guestsEnd)
{
    // Per-guest decanting
    std::vector<std::vector<std::shared_ptr<const Guest>>> partition;
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        partition.push_back(std::vector{ *guestsBegin });
    }
    return partition;
}

inline bool guestsHaveSharedPage(const Guest &guest1, const Guest &guest2)
{
    const auto &smaller = guest1.pages.size() < guest2.pages.size() ? guest1.pages : guest2.pages;
    const auto &larger = guest1.pages.size() < guest2.pages.size() ? guest2.pages : guest1.pages;

    return std::ranges::any_of(smaller.begin(), smaller.end(),
                               [&](const auto &page) { return larger.contains(page); });
}

template <SharedPtrIterator<const Guest> GuestIt>
static std::vector<std::shared_ptr<const Guest>>
shareGraphDFS(GuestIt cur, GuestIt guestsEnd,
              std::unordered_set<std::shared_ptr<const Guest>> &visited)
{
    std::vector<std::shared_ptr<const Guest>> component;
    visited.insert(*cur);
    component.push_back(*cur);

    for (auto it = cur; it != guestsEnd; ++it) {
        if (!visited.contains(*it) && guestsHaveSharedPage(**cur, **it)) {
            // TODO could make this iterative
            shareGraphDFS(it, guestsEnd, visited);
        }
    }

    return component;
}

template <SharedPtrIterator<const Guest> GuestIt>
std::vector<std::vector<std::shared_ptr<const Guest>>>
makeShareGraphComponentGuestPartitions(GuestIt guestsBegin, GuestIt guestsEnd)
{
    std::vector<std::vector<std::shared_ptr<const Guest>>> result;
    std::unordered_set<std::shared_ptr<const Guest>> visited;

    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        if (visited.contains(*guestsBegin)) {
            continue;
        }

        const auto component = shareGraphDFS(guestsBegin, guestsEnd, visited);
        result.push_back(std::move(component));
    }

    return result;
}

}  // namespace vmp

#endif  // VMP_SOLVERUTILS_H

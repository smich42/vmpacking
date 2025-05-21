#ifndef VMP_SOLVERUTILS_H
#define VMP_SOLVERUTILS_H

#include <vmp_solverutils.h>

#include <ranges>
#include <unordered_set>
#include <vmp_guest.h>
#include <vmp_host.h>
#include <vmp_commontypes.h>

namespace vmp
{

double calculateRelSize(const Guest &guest, const std::unordered_map<int, int> &pageFreq);

double calculateSizeRelRatio(const Guest &guest, const std::unordered_map<int, int> &pageFreq);

double calculateOpportunityAwareEfficiency(const Guest &guest, const std::shared_ptr<Host> &host,
                                           const std::vector<std::shared_ptr<Host>> &allHosts);

template <SharedPtrIterator<const Guest> GuestIt>
void decantGuests(
    std::vector<std::shared_ptr<Host>> &hosts,
    std::vector<std::vector<std::shared_ptr<const Guest>>> (*partitionGuests)(GuestIt, GuestIt))
{
    for (auto leftIt = hosts.begin(); leftIt != hosts.end(); ++leftIt) {
        const auto &leftHost = *leftIt;

        for (auto rightIt = leftIt + 1; rightIt != hosts.end(); ++rightIt) {
            const auto &rightHost = *rightIt;
            const auto rightGuests = rightHost->getGuests();

            const auto partitions = partitionGuests(rightGuests.begin(), rightGuests.end());

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
    std::erase_if(hosts, [](const auto &host) { return host->getGuests().empty(); });
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
        for (const auto &guest : (*hostsBegin)->getGuests()) {
            for (const auto &page : guest->pages) {
                ++frequencies[page];
            }
        }
    }
    return frequencies;
}

template <SharedPtrIterator<const Guest> GuestIt>
std::vector<std::vector<std::shared_ptr<const Guest>>>
partitionAllGuestsTogether(GuestIt guestsBegin, GuestIt guestsEnd)
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
partitionGuestsIndividually(GuestIt guestsBegin, GuestIt guestsEnd)
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
std::vector<std::vector<std::shared_ptr<const Guest>>>
partitionConnectedGuestsTogether(GuestIt guestsBegin, GuestIt guestsEnd)
{
    std::vector<std::vector<std::shared_ptr<const Guest>>> result;
    std::unordered_set<std::shared_ptr<const Guest>> guestsVisited;

    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        if (guestsVisited.contains(*guestsBegin)) {
            continue;
        }

        std::vector<std::shared_ptr<const Guest>> component;
        std::queue<std::shared_ptr<const Guest>> guestsToVisit;
        guestsToVisit.push(*guestsBegin);

        while (!guestsToVisit.empty()) {
            const auto guest = guestsToVisit.front();
            guestsToVisit.pop();

            component.push_back(guest);
            guestsVisited.insert(guest);

            for (auto it = std::next(guestsBegin); it != guestsEnd; ++it) {
                const auto childCandidate = *it;

                if (!guestsVisited.contains(childCandidate) &&
                    guestsHaveSharedPage(*childCandidate, *guest)) {
                    guestsToVisit.push(childCandidate);
                }
            }
        }

        result.push_back(std::move(component));
    }

    return result;
}

template <SharedPtrIterator<const Guest> GuestIt>
void decantGuestByAllPartitioners(std::vector<std::shared_ptr<Host>> &hosts)
{
    decantGuests<GuestIt>(hosts, partitionAllGuestsTogether<GuestIt>);
    decantGuests<GuestIt>(hosts, partitionConnectedGuestsTogether<GuestIt>);
    decantGuests<GuestIt>(hosts, partitionGuestsIndividually<GuestIt>);
}

struct TreeLowerBounds
{
    size_t size;   // The total number of pages to pack a subtree
    size_t count;  // The number of hosts required to pack the subtree

    TreeLowerBounds(const size_t size, const size_t count) : size(size), count(count) {}
};

std::unordered_map<size_t, TreeLowerBounds>
calculateAllSubtreeLowerBounds(const TreeInstance &instance);

}  // namespace vmp

#endif  // VMP_SOLVERUTILS_H

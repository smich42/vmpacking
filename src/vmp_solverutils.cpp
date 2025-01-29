#include <vmp_solverutils.h>

#include <vmp_guest.h>
#include <vmp_host.h>

#include <cmath>
#include <numeric>
#include <queue>
#include <unordered_map>

namespace vmp
{

double calculateRelSize(const Guest &guest, const std::unordered_map<int, int> &pageFreq)
{
    return std::accumulate(guest.pages.begin(), guest.pages.end(), 0.0,
                           [&](const double sum, const int page) {
                               return sum + 1.0 / static_cast<double>(pageFreq.at(page));
                           });
}

double calculateSizeRelRatio(const Guest &guest, const std::unordered_map<int, int> &pageFreq)
{
    return static_cast<double>(guest.getPageCount()) / calculateRelSize(guest, pageFreq);
}

double calculateLocalityScore(const Guest &guest, const std::shared_ptr<Host> &host,
                              const std::vector<std::shared_ptr<Host>> &allHosts)
{
    const size_t pagesOnHost = guest.countPagesOn(*host);

    size_t minDifferenceWithOtherHost = std::numeric_limits<size_t>::max();
    for (const auto &otherHost : allHosts) {
        if (host != otherHost) {
            minDifferenceWithOtherHost =
                std::min(minDifferenceWithOtherHost, otherHost->countPagesNotOn(guest));
        }
    }

    return static_cast<double>(pagesOnHost + minDifferenceWithOtherHost) /
           std::sqrt(guest.getPageCount());
}

double countGuestPagesPlaced(const Guest &guest,
                             const std::vector<std::shared_ptr<const Host>> &hosts)
{
    const auto frequencies = calculatePageFrequencies(hosts.begin(), hosts.end());

    int count = 0;
    for (int page : guest.pages) {
        if (frequencies.contains(page)) {
            ++count;
        }
    }

    return count;
}

std::unordered_map<size_t, TreeLowerBounds>
calculateAllSubtreeLowerBounds(const TreeInstance &instance)
{
    std::unordered_map<size_t, TreeLowerBounds> res;

    // The capacity of each of those hosts (as we go deeper down the tree, we
    // subtract from this capacity measurement to reflect that the nodes we've
    // visited must have been packed)
    std::unordered_map<size_t, size_t> capacities;

    // For later bottom-up traversal
    std::unordered_map<size_t, size_t> unvisitedChildCounts;

    std::queue<size_t> bottomUpNodesToVisit;
    std::queue<size_t> topDownNodesToVisit;

    // To calculate capacities we must go top-down
    topDownNodesToVisit.push(TreeInstance::getRootNode());
    capacities[TreeInstance::getRootNode()] = instance.getCapacity();

    while (!topDownNodesToVisit.empty()) {
        const size_t node = topDownNodesToVisit.front();
        topDownNodesToVisit.pop();

        const size_t weight = instance.getNodePages(node).size();

        const auto &children = instance.getNodeChildren(node);
        for (const size_t child : children) {
            capacities[child] = capacities[node] - weight;
            topDownNodesToVisit.push(child);
        }
        if ((unvisitedChildCounts[node] = children.size()) == 0) {
            bottomUpNodesToVisit.push(node);
        }
    }

    // To calculate size and weight we must go bottom-up
    while (!bottomUpNodesToVisit.empty()) {
        const size_t node = bottomUpNodesToVisit.front();
        bottomUpNodesToVisit.pop();

        const size_t parent = instance.getNodeParent(node);
        if (node != TreeInstance::getRootNode() && --unvisitedChildCounts[parent] == 0) {
            bottomUpNodesToVisit.push(parent);
        }

        const size_t weight = instance.getNodePages(node).size();

        if (instance.nodeIsLeaf(node)) {
            res.emplace(node, TreeLowerBounds(weight, 1));
            continue;
        }

        size_t childrenTotalSize = 0;
        for (const size_t child : instance.getNodeChildren(node)) {
            childrenTotalSize += res.at(child).size;
        }

        const size_t count = std::ceil(static_cast<double>(childrenTotalSize) /
                                       static_cast<double>(capacities[node] - weight));
        const size_t size = childrenTotalSize + count * weight;

        res.emplace(node, TreeLowerBounds(size, count));
    }

    return res;
}

}  // namespace vmp
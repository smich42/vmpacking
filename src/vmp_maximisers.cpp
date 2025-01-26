#include <vmp_maximisers.h>

#include <vmp_clustertreeinstance.h>

#include <cassert>
#include <iostream>
#include <queue>
#include <unordered_set>

namespace vmp
{
Packing maximiseByLocalSearch(const GeneralInstance &instance, const size_t allowedHostCount,
                              Host (*oneHostMaximiser)(GuestProfitVecIt, GuestProfitVecIt, size_t),
                              const double oneHostApproxRatio, const double epsilon)
{
    std::vector initialPlacements(instance.guestCount(), false);

    std::vector<std::shared_ptr<Host>> hosts;
    hosts.reserve(allowedHostCount);

    for (size_t i = 0; i < allowedHostCount; ++i) {
        const auto host = std::make_shared<Host>(instance.capacity);

        for (size_t j = 0; j < instance.guestCount(); ++j) {
            if (!initialPlacements[j] && host->accommodatesGuest(*instance.guests[j])) {
                host->addGuest(instance.guests[j]);
                initialPlacements[j] = true;
            }
        }

        hosts.push_back(host);
    }

    // Fleischer, et al. iteration count for achieving
    // (oneHostApproxRatio/(oneHostApproxRatio + 1) + epsilon) approximation
    const size_t iterations = std::abs(static_cast<int>(
        std::ceil(1.0 / oneHostApproxRatio * static_cast<double>(allowedHostCount) *
                  std::log(1.0 / epsilon))));

    for (size_t _ = 0; _ < iterations; ++_) {
        int maxImprovement = 0;
        size_t mostImprovableIndex = 0;
        std::shared_ptr<Host> mostImprovingCandidate;

        for (size_t i = 0; i < hosts.size(); i++) {
            std::vector<std::pair<std::shared_ptr<const Guest>, int>> guestsWithValues;

            for (const auto &guest : instance.guests) {
                int value = 1;
                for (const auto &host : hosts) {
                    if (host != hosts[i] && host->hasGuest(guest)) {
                        value = 0;
                        break;
                    }
                }
                guestsWithValues.emplace_back(guest, value);
            }

            Host candidate = oneHostMaximiser(guestsWithValues.begin(), guestsWithValues.end(),
                                              instance.capacity);
            const int improvement =
                static_cast<int>(candidate.guestCount()) - static_cast<int>(hosts[i]->guestCount());

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

            for (const auto &guest : hosts[mostImprovableIndex]->guests)
                if (host->hasGuest(guest)) {
                    host->removeGuest(guest);
                }
        }
    }

    size_t count = 0;
    for (const auto &host : hosts) {
        count += host->guestCount();
    }

    // Clean up as there is no guarantee that we will utilise all bins
    std::erase_if(hosts, [](const std::shared_ptr<Host> &host) { return host->guestCount() == 0; });
    return Packing(hosts);
}

/* Suppose s is a subset of the nodes of a cluster n, and p is a profit
 * target. cost[n, s, j, p] is the least page count by which we can achieve
 * >= p by packing s on the server and optionally using nodes from the
 * first j children clusters of n.*/
struct ProfitScenario
{
    size_t cluster;        // scenario considers the subtree rooted here
    size_t selectionMask;  // the nodes selected from this cluster
    size_t childCount;  // the subtrees rooted at children [0...childCount-1] of this cluster can be
                        // used
    size_t profitTarget;

    bool operator==(const ProfitScenario &) const = default;

    [[nodiscard]] size_t hash() const
    {
        size_t h = std::hash<size_t>{}(cluster);
        h ^= std::hash<size_t>{}(selectionMask);
        h ^= std::hash<size_t>{}(childCount);
        h ^= std::hash<size_t>{}(profitTarget);
        return h;
    }
};

struct Cost
{
    size_t pageCount;
    std::vector<std::shared_ptr<const Guest>> guests;

    Cost() : pageCount(std::numeric_limits<int>::max()) {}
    Cost(const size_t pageCount, const std::vector<std::shared_ptr<const Guest>> &guests)
        : pageCount(pageCount), guests(guests)
    {
    }

    void setFromSelection(const std::set<size_t> &selection, const std::set<int> &pages,
                          const ClusterTreeInstance &instance)
    {
        pageCount = static_cast<int>(pages.size());
        guests.clear();
        guests.reserve(selection.size());

        for (const size_t selected : selection) {
            if (instance.nodeIsLeaf(selected)) {
                guests.push_back(instance.nodeGuest(selected));
            }
        }
    }

    void setFromCombinationOfDisjoint(const Cost &a, const Cost &b)
    {
        pageCount = a.pageCount + b.pageCount;

        guests.clear();
        guests.reserve(a.guests.size() + b.guests.size());
        guests.insert(guests.end(), a.guests.begin(), a.guests.end());
        guests.insert(guests.end(), b.guests.begin(), b.guests.end());
    }

    Cost(const Cost &) = default;
    Cost(Cost &&) noexcept = default;
    Cost &operator=(const Cost &) = default;
    Cost &operator=(Cost &&) noexcept = default;
    ~Cost() = default;
};

static std::pair<std::set<size_t>, std::set<int>>
selectNodesByMask(const ClusterTreeInstance &instance, const std::vector<size_t> &pool,
                  const uint64_t mask)
{
    std::set<int> pages;
    std::set<size_t> selection;

    for (uint64_t i = 0; i < pool.size(); i++) {
        if (mask & 1ULL << i) {
            const auto &nodePages = instance.nodePages(pool[i]);
            pages.insert(nodePages.begin(), nodePages.end());
            selection.insert(pool[i]);
        }
    }
    return { selection, pages };
}

static size_t makeAccessibleChildrenMask(const std::vector<size_t> &nodes,
                                         const std::vector<size_t> &parentCandidates,
                                         const ClusterTreeInstance &instance)
{
    size_t accessibleChildrenMask = 0;
    for (int i = 0; i < nodes.size(); ++i) {
        const auto &trueParents = instance.nodeParents(nodes[i]);

        for (const size_t parentCandidate : parentCandidates) {
            if (std::ranges::find(trueParents, parentCandidate) != trueParents.end()) {
                accessibleChildrenMask |= 1ULL << i;
                break;
            }
        }
    }
    return accessibleChildrenMask;
}

static bool checkAllAccessible(const size_t childrenMask, const size_t accessibleMask)
{
    return (childrenMask & ~accessibleMask) == 0;
}

static std::optional<Cost> findLowestCostAccessibleSelection(const auto &costs,
                                                             const size_t cluster,
                                                             const size_t accessibleMask,
                                                             const size_t profitTarget,
                                                             const ClusterTreeInstance &instance)
{
    const std::vector<size_t> &nodes = instance.clusterNodes(cluster);
    assert(nodes.size() < 64);

    const Cost *lowestCost = nullptr;

    for (uint64_t selectionMask = 0; selectionMask < 1ULL << nodes.size(); ++selectionMask) {
        if (!checkAllAccessible(selectionMask, accessibleMask)) {
            continue;
        }

        const auto selection = selectNodesByMask(instance, nodes, selectionMask).first;
        const size_t degree = instance.clusterChildren(cluster).size();
        // Assume we have already processed the child nodes by topological sort
        const Cost *cost = &costs.at({ cluster, selectionMask, degree, profitTarget });

        if (!lowestCost || cost->pageCount < lowestCost->pageCount) {
            lowestCost = cost;
        }
    }
    return lowestCost == nullptr ? std::nullopt : std::make_optional<Cost>(*lowestCost);
}

static std::optional<Cost> findMostProfitableScenarioAtRoot(const auto &costs,
                                                            const ClusterTreeInstance &instance)
{
    const size_t root = ClusterTreeInstance::rootCluster();
    const size_t rootDegree = instance.clusterChildren(root).size();
    size_t bestProfit = 0;
    const Cost *bestCost = nullptr;

    for (auto &[key, value] : costs) {
        if (key.cluster == root && key.childCount == rootDegree && key.profitTarget > bestProfit &&
            value.pageCount <= instance.capacity) {
            bestProfit = key.profitTarget;
            bestCost = &value;
        }
    }

    return bestCost == nullptr ? std::nullopt : std::make_optional<Cost>(*bestCost);
}

Host maximiseOneHostByClusterTree(const ClusterTreeInstance &instance)
{
    std::unordered_map<ProfitScenario, Cost,
                       decltype([](const ProfitScenario &k) { return k.hash(); })>
        costs;

    // A simple upper bound is packing all the nodes
    // TODO other upper bounds?
    const int maxProfit = static_cast<int>(instance.leafNodes().size());

    // Topological sort:
    // Track # of unvisited children of each cluster
    // We add a cluster to the frontier only once it has 0 unvisited children
    // As we need the cost table to have been computer for all its child entries
    std::unordered_map<size_t, size_t> unvisitedClusterChildCount;
    std::queue<size_t> clustersToVisit;

    for (size_t cluster = 0; cluster < instance.clusterCount(); ++cluster) {
        if (instance.clusterIsLeaf(cluster)) {
            clustersToVisit.push(cluster);
        }
        else {
            unvisitedClusterChildCount[cluster] = instance.clusterChildren(cluster).size();
        }
    }

    while (!clustersToVisit.empty()) {
        const size_t cluster = clustersToVisit.front();
        // We consider one cluster at a time, bottom-up
        clustersToVisit.pop();

        // Decrement the unvisited child count of each parent
        const size_t parent = instance.clusterParent(cluster);
        if (--unvisitedClusterChildCount[parent] == 0) {
            clustersToVisit.push(parent);
        }

        const std::vector<size_t> &curNodes = instance.clusterNodes(cluster);
        const auto &curChildren = instance.clusterChildren(cluster);

        assert(curNodes.size() < 64);
        // Begin by considering every one of 2^(node count) choices of nodes from this cluster
        for (uint64_t curMask = 0; curMask < 1ULL << curNodes.size(); ++curMask) {
            const auto [curSelection, curSelectionPages] =
                selectNodesByMask(instance, curNodes, curMask);
            // If at a leaf cluster, we have made a profit equal to the size of the selection choice
            // Otherwise no profit is actualised
            const bool curIsLeaf = curChildren.empty();
            const size_t profitMade = curIsLeaf ? std::popcount(curMask) : 0;

            for (size_t profitTarget = 0; profitTarget <= maxProfit; ++profitTarget) {
                // Initialise:
                // cost[n, s, 0, p] = if (sum profit in s) >= p then |union pages in s| else +inf
                ProfitScenario curKey{ cluster, curMask, 0, profitTarget };
                if (curSelectionPages.size() > instance.capacity || profitMade < profitTarget) {
                    costs[curKey] = Cost();
                }
                else {
                    costs[curKey].setFromSelection(curSelection, curSelectionPages, instance);
                }

                // Allow taking from the first j children at a time
                for (size_t j = 1; j <= curChildren.size(); ++j) {
                    // We will compute cost[n, s, j, p]
                    curKey = { cluster, curMask, j, profitTarget };
                    // Try to do better than with j - 1 children
                    costs[curKey] = costs.at({ cluster, curMask, j - 1, profitTarget });

                    // Only those nodes in the child cluster that have at least one parent in the
                    // current selection are accessible As the mask must be over all the cluster's
                    // TODO: maybe use the set itself as key
                    const size_t newChild = curChildren[j - 1];
                    const std::vector<size_t> &newChildNodes = instance.clusterNodes(newChild);
                    const size_t accessibleChildrenMask =
                        makeAccessibleChildrenMask(newChildNodes, curNodes, instance);

                    // Try to make `profitComplement` profit from the newly considered child cluster
                    for (size_t profitComplement = 0; profitComplement <= profitTarget;
                         ++profitComplement) {
                        const Cost bestChildCost =
                            findLowestCostAccessibleSelection(
                                costs, newChild, accessibleChildrenMask, profitComplement, instance)
                                .value_or(Cost());
                        const Cost &prevCost =
                            costs.at({ cluster, curMask, j - 1, profitTarget - profitComplement });

                        if (bestChildCost.pageCount == std::numeric_limits<int>::max() ||
                            prevCost.pageCount == std::numeric_limits<int>::max()) {
                            continue;
                        }

                        const size_t candidatePageCount =
                            prevCost.pageCount + bestChildCost.pageCount;

                        if (candidatePageCount <= instance.capacity &&
                            candidatePageCount < costs.at(curKey).pageCount) {
                            costs[curKey].setFromCombinationOfDisjoint(prevCost, bestChildCost);
                        }
                    }
                }
            }
        }
    }

    Host host(instance.capacity);

    const auto bestCost = findMostProfitableScenarioAtRoot(costs, instance);
    if (!bestCost.has_value()) {
        return host;
    }

    for (const auto &guest : bestCost->guests) {
        host.addGuest(guest);
    }

    return host;
}

}  // namespace vmp
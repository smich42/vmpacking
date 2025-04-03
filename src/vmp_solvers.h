#ifndef VMP_SOLVERS_H
#define VMP_SOLVERS_H

#include <cassert>
#include <vmp_generalinstance.h>
#include <vmp_packing.h>
#include <vmp_solverutils.h>

namespace vmp
{

/**
 * Solves an instance of VM-PACK by Next Fit.
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByNextFit(const GeneralInstance &instance, bool decant = false);

/**
 * Solves a `TreeInstance` by Next Fit.
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByNextFit(const TreeInstance &instance, bool decant);

/**
 * Solves a `ClusterTreeInstance` by Next Fit.
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByNextFit(const ClusterTreeInstance &instance, bool decant);

/**
 * Solves an instance of VM-PACK by First Fit
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByFirstFit(const GeneralInstance &instance);

/**
 * Solves a `TreeInstance` by First Fit.
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByFirstFit(const TreeInstance &instance);

/**
 * Solves a `ClusterTreeInstance` by First Fit.
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByFirstFit(const ClusterTreeInstance &instance);

/**
 * Solves an instance of VM-PACK by "Best Fusion" of Grange, et
 * al. (2021)
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByEfficiency(const GeneralInstance &instance, bool decant);

/**
 * Solves a Cluster Tree instance of VM-PACK by "Best Fusion" of Grange, et
 * al. (2021)
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByEfficiency(const ClusterTreeInstance &instance, bool decant);

/**
 * Solves an instance of VM-PACK by "Overload-and-Remove" of Grange, et
 * al. (2021)
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByOverloadAndRemove(const GeneralInstance &instance, bool decant);

/**
 * Solves a Cluster Tree instance of VM-PACK by "Overload-and-Remove" of Grange, et
 * al. (2021)
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByOverloadAndRemove(const ClusterTreeInstance &instance, bool decant);

/**
 * Solves an instance of VM-PACK by method similar to Shao & Liang (2023).
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByOpportunityAwareEfficiency(const GeneralInstance &instance, bool decant);

/**
 * Solves an instance of VM-PACK by searching for the minimum number of bins
 * that yield a complete packing using the given maximisation algorithm.
 *
 * O(log(G) * T_maximiser)
 *
 * @param instance the instance to solve
 * @param maximiser the n-host maximiser
 * @param maximiser_is_dynamic_sized whether the maximiser will produce a minimal packing when given
 * unlimited allowance
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing with the least maxHosts
 */
template <typename InstanceType>
    requires Instance<InstanceType>
Packing solveByMaximiser(
    const InstanceType &instance,
    const std::function<Packing(const InstanceType &instance, size_t maxHosts)> &maximiser,
    const bool maximiser_is_dynamic_sized = false, const bool decant = false)
{
    std::optional<Packing> bestPacking;

    if (maximiser_is_dynamic_sized) {
        bestPacking = maximiser(instance, std::numeric_limits<size_t>::max());
    }
    else {
        size_t minHosts = 1;
        size_t maxHosts = instance.getGuests().size();

        while (minHosts <= maxHosts) {
            const size_t allowedHostCount = minHosts + (maxHosts - minHosts) / 2;
            Packing candidate = maximiser(instance, allowedHostCount);

            if (candidate.getGuestCount() == instance.getGuests().size()) {
                bestPacking = std::move(candidate);
                maxHosts = allowedHostCount - 1;
            }
            else {
                minHosts = allowedHostCount + 1;
            }
        }
    }

    if (!bestPacking) {
        throw std::runtime_error("No valid packing found; is a guest larger than the capacity?");
    }

    auto hosts = bestPacking->getHosts();
    if (decant) {
        using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
        decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
        decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
    }

    return Packing(hosts);
}

/**
 * Solves VM-PACK by the Sinderal, et al. (2011) greedy algorithm on the tree model.
 *
 * O((G^2 * P) + N^2), where G is the number of guests, P the maximum number of pages on one guest
 * and N the number of nodes
 *
 * @param instance the instance to solve
 * @param intermediateSolver the intermediate solver with which to pack each extracted subtree
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
template <SharedPtrIterator<const Guest> GuestIt>
Packing solveByTree(const TreeInstance &instance,
                    void (*intermediateSolver)(size_t, GuestIt, GuestIt), const bool decant = true)
{
    TreeInstance workingInstance = instance;

    std::vector<std::shared_ptr<Host>> hosts;

    while (true) {
        const auto lowerBounds = calculateAllSubtreeLowerBounds(workingInstance);

        if (lowerBounds.at(TreeInstance::getRootNode()).count == 1) {
            const auto &guests = workingInstance.getGuests();
            if (guests.empty()) {
                break;
            }

            Host host(workingInstance.getCapacity());
            host.addGuests(guests.begin(), guests.end());

            hosts.push_back(std::make_shared<Host>(std::move(host)));
            break;
        }

        size_t minNode = std::numeric_limits<size_t>::max();
        size_t minNodeCount = std::numeric_limits<size_t>::max();

        for (const auto &[node, bounds] : lowerBounds) {
            if (bounds.count <= 1) {
                continue;
            }

            const auto &children = workingInstance.getNodeChildren(node);
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

        const auto &guestsToPack = workingInstance.getSubtreeGuests(minNode);
        intermediateSolver(instance.getCapacity(), guestsToPack.begin(), guestsToPack.end(), hosts);
        if (decant) {
            using SetGuestIt = std::unordered_set<std::shared_ptr<const Guest>>::iterator;
            decantGuests<SetGuestIt>(hosts, partitionAllGuestsTogether<SetGuestIt>);
            decantGuests<SetGuestIt>(hosts, partitionConnectedGuestsTogether<SetGuestIt>);
            decantGuests<SetGuestIt>(hosts, partitionGuestsIndividually<SetGuestIt>);
        }
        if (minNode == TreeInstance::getRootNode()) {
            break;
        }

        workingInstance.removeSubtree(minNode);
    }

    return Packing(hosts);
}

/**
 * Solves the instance by reduction to the n-host maximisation problem, then approximate reduction
 * to the 1-host maximisation problem, which is approximated by Li, et al. (2009), or Rampersaud &
 * Grosu (2014) if initialSubsetSize = 1.
 *
 * @param instance the instance to solve
 * @param initialSubsetSize place guests by computing the efficiency of each possible guest subset
 * of this size
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByLocalSubsetEfficiency(const GeneralInstance &instance, int initialSubsetSize,
                                     bool decant);

/**
 * Solves the instance by reduction to the n-host maximisation problem, then approximate reduction
 * to the 1-host maximisation problem, which is approximated by the Sinderal, et al. (2011) DP
 * algorithm on the cluster-tree model
 *
 * @param instance the instance to solve
 * @param decant whether to apply the decanting post-treatment
 * @return a valid packing
 */
Packing solveByLocalClusterTree(const ClusterTreeInstance &instance, bool decant);

}  // namespace vmp

#endif  // VMP_SOLVERS_H

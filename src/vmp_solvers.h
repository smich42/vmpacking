#ifndef VMP_SOLVERS_H
#define VMP_SOLVERS_H

#include <vmp_generalinstance.h>
#include <vmp_packing.h>

namespace vmp
{

/**
 * Solves an instance of VM-PACK by Next Fit.
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByNextFit(const GeneralInstance &instance);

/**
 * Solves an instance of VM-PACK by First Fit
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByFirstFit(const GeneralInstance &instance);

/**
 * Solves an instance of VM-PACK by "Best Fusion" of Grange, et
 * al. (2021)
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByBestFusion(const GeneralInstance &instance);

/**
 * Solves an instance of VM-PACK by "Overload-and-Remove" of Grange, et
 * al. (2021)
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByOverloadAndRemove(const GeneralInstance &instance);

/**
 * Solves an instance of VM-PACK by method similar to Shao & Liang (2023).
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByLocalityScore(const GeneralInstance &instance);

/**
 * Solves an instance of VM-PACK by searching for the minimum number of bins
 * that yield a complete packing using the given maximisation algorithm.
 *
 * O(log(G) * T_maximiser)
 *
 * @param instance the instance to solve
 * @param maximiser the n-host maximiser
 * @return a valid packing with the least maxHosts
 */
template <typename InstanceType>
    requires Instance<InstanceType>
Packing solveByMaximiser(
    const InstanceType &instance,
    const std::function<Packing(const InstanceType &instance, size_t maxHosts)> &maximiser)
{
    size_t minHosts = 1;
    size_t maxHosts = instance.getGuests().size();

    std::optional<Packing> bestPacking;

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

    if (!bestPacking) {
        throw std::runtime_error("No valid packing found; is a guest larger than the capacity?");
    }

    return *bestPacking;
}

/**
 * Solves VM-PACK by the Sinderal, et al. (2011) greedy algorithm on the tree model.
 *
 * O((G^2 * P) + N^2), where G is the number of guests, P the maximum number of pages on one guest
 * and N the number of nodes
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveBySimpleTree(const TreeInstance &instance);

/**
 * Solves the instance by reduction to the n-host maximisation problem, then approximate reduction
 * to the 1-host maximisation problem, which is approximated by Li, et al. (2009), or Rampersaud &
 * Grosu (2014) if initialSubsetSize = 1
 *
 * @param instance the instance to solve
 * @param initialSubsetSize place guests by computing the efficiency of each possible guest subset
 * of this size
 * @return a valid packing
 */
Packing solveBySubsetEfficiency(const GeneralInstance &instance, int initialSubsetSize,
                                double epsilon = 0.01);

/**
 * Solves the instance by reduction to the n-host maximisation problem, then approximate reduction
 * to the 1-host maximisation problem, which is approximated by the Sinderal, et al. (2011) DP
 * algorithm on the cluster-tree model
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
Packing solveByClusterTree(const ClusterTreeInstance &instance, double epsilon = 0.01);

}  // namespace vmp

#endif  // VMP_SOLVERS_H

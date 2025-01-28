#ifndef VMP_SOLVERS_H
#define VMP_SOLVERS_H

#include <vmp_clustertreeinstance.h>
#include <vmp_generalinstance.h>
#include <vmp_packing.h>

namespace vmp
{

/**
 * Solves an instance of VM-PACK by Next Fit
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
 * Solves an instance of VM-PACK by searching for the number of bins which will
 * yield a complete packing using the given maximisation algorithm.
 *
 * @param instance the instance to solve
 * @return a valid packing
 */
template <typename InstanceType>
    requires Instance<InstanceType>
Packing solveByMaximiser(const InstanceType &instance,
                         Packing (*maximiser)(const InstanceType &instance, size_t maxHosts))
{
    size_t minHosts = 0;
    size_t maxHosts = instance.getGuests().size() + 1;

    std::shared_ptr<Packing> bestPacking;

    while (minHosts <= maxHosts) {
        const size_t allowedHostCount = (minHosts + maxHosts) / 2;
        Packing candidate = maximiser(instance, allowedHostCount);

        if (candidate.countGuests() == instance.getGuests().size()) {
            bestPacking = std::make_shared<Packing>(std::move(candidate));
            maxHosts = allowedHostCount - 1;
        }
        else {  // Not all guests could be packed
            minHosts = allowedHostCount + 1;
        }
    }

    if (bestPacking == nullptr) {
        throw std::runtime_error("No valid packing found; is a guest larger than the capacity?");
    }
    return *bestPacking;
}

Packing solveSimpleTree(const ClusterTreeInstance &instance);

}  // namespace vmp

#endif  // VMP_SOLVERS_H

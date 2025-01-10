#ifndef VMP_SOLVERS_H
#define VMP_SOLVERS_H

#include <vmp_clustertreeinstance.h>
#include <vmp_generalinstance.h>
#include <vmp_packing.h>
#include <vmp_treeinstance.h>

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
Packing solveByMaximiser(const GeneralInstance &instance,
                         Packing (*maximiser)(const GeneralInstance &instance,
                                              size_t allowedHostCount));

Packing solveTree(const TreeInstance &instance);

Packing solveClusterTree(const ClusterTreeInstance &instance);

}  // namespace vmp

#endif  // VMP_SOLVERS_H

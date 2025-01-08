#ifndef VMP_SOLVERS_H
#define VMP_SOLVERS_H

#include <vmp_clustertreeinstance.h>
#include <vmp_generalinstance.h>
#include <vmp_packing.h>
#include <vmp_treeinstance.h>

namespace vmp
{

Packing solveByNextFit(const GeneralInstance &instance);

Packing solveByFirstFit(const GeneralInstance &instance);

Packing solveByBestFusion(const GeneralInstance &instance);

Packing solveByOverloadAndRemove(const GeneralInstance &instance);

Packing solveByMaximiser(const GeneralInstance &instance,
                         Packing (*maximiser)(const GeneralInstance &instance,
                                              size_t allowedHostCount));

Packing solveTree(const TreeInstance &instance);

Packing solveClusterTree(const ClusterTreeInstance &instance);

}  // namespace vmp

#endif  // VMP_SOLVERS_H

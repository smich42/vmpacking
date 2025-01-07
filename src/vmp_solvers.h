#ifndef VMP_SOLVERS_H
#define VMP_SOLVERS_H

#include <vmp_generalinstance.h>
#include <vmp_packing.h>

namespace vmp
{

Packing solveByNextFit(const std::shared_ptr<GeneralInstance> &instance);

Packing solveByFirstFit(const std::shared_ptr<GeneralInstance> &instance);

Packing solveByBestFusion(const std::shared_ptr<GeneralInstance> &instance);

Packing
solveByOverloadAndRemove(const std::shared_ptr<GeneralInstance> &instance);

Packing solveByMaximiser(
    const std::shared_ptr<GeneralInstance> &instance,
    const std::function<Packing(std::vector<std::shared_ptr<Guest>> guests,
                                size_t capacity, size_t maxHosts)> &maximiser);

}  // namespace vmp

#endif  // VMP_SOLVERS_H

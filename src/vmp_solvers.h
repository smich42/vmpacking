#ifndef VMP_SOLVERS_H
#define VMP_SOLVERS_H

#include <vmp_packing.h>

namespace vmp
{

Packing solveNextFit(const std::shared_ptr<GeneralInstance> &instance);

Packing solveFirstFit(const std::shared_ptr<GeneralInstance> &instance);

Packing solveBestFusion(const std::shared_ptr<GeneralInstance> &instance);

Packing solveOverloadAndRemove(const std::shared_ptr<GeneralInstance> &instance);

}  // namespace vmp

#endif  // VMP_SOLVERS_H

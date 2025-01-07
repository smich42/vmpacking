#ifndef VMP_MAXIMISERS_H
#define VMP_MAXIMISERS_H

#include <vmp_packing.h>

namespace vmp
{

Packing maximiseByLocalSearch(
    const GeneralInstance &instance,
    std::function<Host(std::vector<std::shared_ptr<Guest>> guests,
                       int capacity)>
        localMaximiser);

}  // namespace vmp

#endif  // VMP_MAXIMISERS_H

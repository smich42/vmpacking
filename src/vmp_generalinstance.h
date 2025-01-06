#ifndef SOLVERS_INSTANCE_H
#define SOLVERS_INSTANCE_H

#include <vector>
#include <vmp_guest.h>

namespace vmp
{

class GeneralInstance
{
  public:
    GeneralInstance(size_t capacity,
                    const std::vector<std::shared_ptr<Guest>> &guests);

    friend std::ostream &operator<<(std::ostream &os,
                                    const GeneralInstance &instance);

    const size_t capacity;
    const std::vector<std::shared_ptr<Guest>> guests;
};

}  // namespace vmp

#endif  // SOLVERS_INSTANCE_H
#ifndef SOLVERS_INSTANCE_H
#define SOLVERS_INSTANCE_H

#include <vector>
#include <vmp_guest.h>

namespace vmp
{

class GeneralInstance
{
  public:
    GeneralInstance(size_t capacity, const std::vector<std::shared_ptr<const Guest>> &guests);

    [[nodiscard]] size_t guestCount() const;

    friend std::ostream &operator<<(std::ostream &os, const GeneralInstance &instance);

    const size_t capacity;
    const std::vector<std::shared_ptr<const Guest>> guests;
};

}  // namespace vmp

#endif  // SOLVERS_INSTANCE_H
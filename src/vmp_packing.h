#ifndef SOLVERS_PACKING_H
#define SOLVERS_PACKING_H

#include <vmp_generalinstance.h>
#include <vmp_host.h>

namespace vmp
{

class Packing
{
  public:
    explicit Packing(std::vector<std::shared_ptr<Host>> hosts);

    Packing(Packing &&other) noexcept = default;

    Packing &operator=(Packing &&other) noexcept = default;

    [[nodiscard]] bool validate() const;
    [[nodiscard]] bool countGuests() const;

    std::unique_ptr<std::vector<std::shared_ptr<Host>>> hosts;
};
}  // namespace vmp
#endif  // SOLVERS_PACKING_H
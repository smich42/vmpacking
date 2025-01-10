#ifndef SOLVERS_PACKING_H
#define SOLVERS_PACKING_H

#include <vmp_generalinstance.h>
#include <vmp_host.h>

namespace vmp
{

class Packing
{
  public:
    explicit Packing(const std::vector<std::shared_ptr<Host>> &hosts);

    Packing(Packing &other) noexcept = default;
    Packing(Packing &&other) noexcept = default;

    Packing &operator=(Packing &&other) noexcept = default;

    [[nodiscard]] bool validate() const;
    [[nodiscard]] size_t countGuests() const;
    [[nodiscard]] size_t hostCount() const;

    std::vector<std::shared_ptr<Host>> hosts;
};
}  // namespace vmp
#endif  // SOLVERS_PACKING_H
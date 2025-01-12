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

    /**
     * Validate the packing against an instance, by checking that:
     * - All guests are present,
     * - No host is overfull; and
     * - No host is empty
     *
     * @param instance the instance against which to validate
     * @return
     */
    [[nodiscard]] bool
    validateForInstance(const GeneralInstance &instance) const;

    [[nodiscard]] size_t countGuests() const;
    [[nodiscard]] size_t hostCount() const;

    std::vector<std::shared_ptr<Host>> hosts;
};

}  // namespace vmp
#endif  // SOLVERS_PACKING_H
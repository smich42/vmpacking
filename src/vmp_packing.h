#ifndef SOLVERS_PACKING_H
#define SOLVERS_PACKING_H

#include <vmp_generalinstance.h>
#include <vmp_host.h>

namespace vmp
{

class Packing
{
  public:
    explicit Packing(const std::shared_ptr<GeneralInstance> &instance,
                     const std::vector<std::shared_ptr<Host>> &hosts);
    [[nodiscard]] bool validate() const;

    const std::shared_ptr<GeneralInstance> instance;
    const std::unique_ptr<std::vector<std::shared_ptr<Host>>> hosts;
};

}  // namespace vmp
#endif  // SOLVERS_PACKING_H
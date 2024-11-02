#ifndef SOLVERS_PACKING_H
#define SOLVERS_PACKING_H

#include <vmp_host.h>
#include <vmp_instance.h>

namespace vmp
{

class Packing
{
  public:
    explicit Packing(const std::shared_ptr<Instance> &instance,
                     const std::vector<std::shared_ptr<Host>> &hosts);
    [[nodiscard]] bool validate() const;

    const std::shared_ptr<Instance> instance;
    const std::unique_ptr<std::vector<std::shared_ptr<Host>>> hosts;
};

}  // namespace vmp
#endif  // SOLVERS_PACKING_H
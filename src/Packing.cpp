#include <Packing.h>

Packing::Packing(const std::shared_ptr<Instance> &instance,
                 const std::vector<std::shared_ptr<Host>> &hosts)
    : instance(instance),
      hosts(std::make_unique<std::vector<std::shared_ptr<Host>>>(hosts))
{
}
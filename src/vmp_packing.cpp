#include <vmp_packing.h>

namespace vmp
{

Packing::Packing(const std::shared_ptr<Instance> &instance,
                 const std::vector<std::shared_ptr<Host>> &hosts)
    : instance(instance),
      hosts(std::make_unique<std::vector<std::shared_ptr<Host>>>(hosts))
{
}

bool Packing::validate() const
{
    return std::ranges::none_of(*hosts, std::mem_fn(&Host::isOverfull));
}

}  // namespace vmp
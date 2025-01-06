#include <vmp_packing.h>

namespace vmp
{

Packing::Packing(const std::shared_ptr<GeneralInstance> &instance,
                 const std::vector<std::shared_ptr<Host>> &hosts)
    : instance(instance),
      hosts(std::make_unique<std::vector<std::shared_ptr<Host>>>(hosts))
{
}

bool Packing::validate() const
{
    return std::ranges::none_of(*hosts, [](const auto &host) {
        return host->isOverfull() || host->guests.empty();
    });
}

}  // namespace vmp
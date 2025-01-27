#include <vmp_packing.h>

namespace vmp
{

Packing::Packing(const std::vector<std::shared_ptr<Host>> &hosts) : hosts(hosts) {}

size_t Packing::countGuests() const
{
    size_t count = 0;
    for (const auto &host : hosts) {
        count += host->getGuestCount();
    }
    return count;
}

size_t Packing::getHostCount() const
{
    return hosts.size();
}

}  // namespace vmp
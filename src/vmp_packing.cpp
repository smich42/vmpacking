#include <vmp_packing.h>

namespace vmp
{

Packing::Packing(const std::vector<std::shared_ptr<Host>> &hosts) : guestCount(0)
{
    for (const auto &host : hosts) {
        addHost(host);
    }
}

void Packing::addHost(const std::shared_ptr<Host> &host)
{
    hosts.push_back(host);
    guestCount += host->getGuestCount();
}

size_t Packing::getGuestCount() const
{
    return guestCount;
}

size_t Packing::getHostCount() const
{
    return hosts.size();
}

}  // namespace vmp
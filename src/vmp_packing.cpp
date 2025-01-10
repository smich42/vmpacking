#include <numeric>
#include <vmp_packing.h>

namespace vmp
{

Packing::Packing(const std::vector<std::shared_ptr<Host>> &hosts)
    : hosts(hosts)
{
}

bool Packing::validate() const
{
    return std::ranges::none_of(hosts, [](const auto &host) {
        return host->isOverfull() || host->guests.empty();
    });
}

size_t Packing::countGuests() const
{
    size_t count = 0;
    for (const auto &host : hosts) {
        count += host->guestCount();
    }
    return count;
}

size_t Packing::hostCount() const
{
    return hosts.size();
}

}  // namespace vmp
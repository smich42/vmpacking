#include <numeric>
#include <vmp_packing.h>

namespace vmp
{

Packing::Packing(const std::vector<std::shared_ptr<Host>> &hosts)
    : hosts(std::vector(hosts))
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
    return std::accumulate(hosts.begin(), hosts.end(), 0,
                           [](const int sum, const auto &host) {
                               return sum + host->guests.size();
                           });
}

}  // namespace vmp
#include <numeric>
#include <vmp_packing.h>

namespace vmp
{

Packing::Packing(std::vector<std::shared_ptr<Host>> hosts)
    : hosts(std::make_unique<std::vector<std::shared_ptr<Host>>>(
          std::move(hosts)))
{
}

bool Packing::validate() const
{
    return std::ranges::none_of(*hosts, [](const auto &host) {
        return host->isOverfull() || host->guests.empty();
    });
}

bool Packing::countGuests() const
{
    return std::accumulate(hosts->begin(), hosts->end(), 0,
                           [](const int sum, const auto &host) {
                               return sum + host->guests.size();
                           });
}

}  // namespace vmp
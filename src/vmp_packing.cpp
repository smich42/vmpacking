#include <vmp_packing.h>

#include <unordered_set>

namespace vmp
{

Packing::Packing(const std::vector<std::shared_ptr<Host>> &hosts) : hosts(hosts) {}

bool Packing::validateForInstance(const GeneralInstance &instance) const
{
    std::unordered_set<std::shared_ptr<const Guest>> placedGuests;
    for (const auto &host : hosts) {
        if (host->guests.empty()) {
            return false;
        }
        if (host->isOverfull()) {
            return false;
        }
        for (const auto &guest : host->guests) {
            placedGuests.insert(guest);
        }
    }

    return std::ranges::all_of(instance.guests.begin(), instance.guests.end(),
                               [&](const auto &guest) { return placedGuests.contains(guest); });
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
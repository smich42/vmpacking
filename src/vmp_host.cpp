#include <vmp_host.h>

#include <ostream>
#include <vmp_guest.h>

namespace vmp
{

Host::Host(const size_t capacity) : capacity(capacity) {}

bool Host::addGuest(const std::shared_ptr<const Guest> &guest)
{
    for (const int page : guest->pages) {
        ++pageFrequencies[page];
    }
    guests.insert(guest);

    return isOverfull();
}

bool Host::removeGuest(const std::shared_ptr<const Guest> &guest)
{
    for (const int page : guest->pages) {
        if (--pageFrequencies[page] == 0) {
            pageFrequencies.erase(page);
        }
    }
    // Erase the entry to avoid maintaining an unnecessary reference
    guests.erase(guest);

    return isOverfull();
}

void Host::clearGuests()
{
    guests.clear();
    pageFrequencies.clear();
}

size_t Host::getCapacity() const
{
    return capacity;
}

const std::set<std::shared_ptr<const Guest>> &Host::getGuests() const
{
    return guests;
}

bool Host::accommodatesGuest(const Guest &guest) const
{
    return countPagesWithGuest(guest) <= capacity;
}

size_t Host::getPageFrequency(const int page) const
{
    return pageFrequencies.contains(page) ? pageFrequencies.at(page) : 0;
}

size_t Host::getUniquePageCount() const
{
    return pageFrequencies.size();
}

size_t Host::countPagesWithGuest(const Guest &guest) const
{
    return getUniquePageCount() + guest.getPageCount() - guest.countPagesOn(*this);
}

size_t Host::countPagesNotOn(const Guest &guest) const
{
    return getUniquePageCount() - guest.countPagesOn(*this);
}

size_t Host::getGuestCount() const
{
    return guests.size();
}

bool Host::isOverfull() const
{
    return pageFrequencies.size() > capacity;
}

bool Host::hasGuest(const std::shared_ptr<const Guest> &guest) const
{
    return guests.contains(guest);
}

}  // namespace vmp
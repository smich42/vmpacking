#include <vmp_host.h>

#include <ostream>
#include <vmp_guest.h>

namespace vmp
{

Host::Host(const int capacity) : capacity(capacity) {}

bool Host::addGuest(const std::shared_ptr<Guest> &guest)
{
    for (const int page : guest->pages) {
        ++pageFreq[page];
    }
    guests.insert(guest);

    return isOverfull();
}

bool Host::removeGuest(const std::shared_ptr<Guest> &guest)
{
    for (const int page : guest->pages) {
        if (--pageFreq[page] == 0) {
            pageFreq.erase(page);
        }
    }
    // Erase after we are done operating on guest
    // as this could be the last reference to it
    guests.erase(guest);

    return isOverfull();
}

void Host::clearGuests()
{
    guests.clear();
    pageFreq.clear();
}

bool Host::accommodatesGuest(const Guest &guest) const
{
    return pageCountWith(guest) <= capacity;
}

size_t Host::pageFrequency(const int page) const
{
    return pageFreq.contains(page) ? pageFreq.at(page) : 0;
}

size_t Host::pageCount() const
{
    return pageFreq.size();
}

size_t Host::pageCountWith(const Guest &guest) const
{
    return pageCount() + guest.pageCount() - guest.pagesOn(*this);
}

size_t Host::guestCount() const
{
    return guests.size();
}

bool Host::isOverfull() const
{
    return pageFreq.size() > capacity;
}

}  // namespace vmp
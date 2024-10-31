#include <Host.h>

#include <Guest.h>

Host::Host(const int capacity) : capacity(capacity) {}

int Host::addGuest(const std::shared_ptr<Guest> &guest)
{
    if (!accommodatesGuest(guest)) {
        return -1;
    }
    for (const int page : guest->pages) {
        ++pageFreq[page];
    }
    guests.push_back(guest);
    return 0;
}

int Host::accommodatesGuest(const std::shared_ptr<Guest> &guest) const
{
    return pageFreq.size() + guest->pages.size() - guest->pagesOn(*this) <=
           capacity;
}
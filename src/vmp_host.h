#ifndef SOLVERS_HOST_H
#define SOLVERS_HOST_H

#include "vmp_iterators.h"

#include <map>
#include <set>
#include <vmp_guest.h>

namespace vmp
{

class Host
{
  public:
    explicit Host(size_t capacity);

    [[nodiscard]] bool accommodatesGuest(const Guest &guest) const;

    template <SharedPtrIterator<const Guest> GuestIt>
    bool accommodatesGuests(GuestIt guestsBegin, GuestIt guestsEnd) const
    {
        return countPagesWithGuests(guestsBegin, guestsEnd) <= capacity;
    }

    template <typename GuestProfitIt,
              typename K = std::shared_ptr<const Guest>, typename V = int>
        requires PairIterator<GuestProfitIt, K, V>
    bool accommodatesGuests(GuestProfitIt guestsBegin,
                            GuestProfitIt guestsEnd) const
    {
        return countPagesWithGuests(guestsBegin, guestsEnd) <= capacity;
    }

    [[nodiscard]] size_t pageFrequency(int page) const;
    [[nodiscard]] size_t pageCount() const;

    template <SharedPtrIterator<const Guest> GuestIt>
    [[nodiscard]] size_t countPagesWithGuests(GuestIt guestsBegin,
                                              GuestIt guestsEnd) const
    {
        std::set<int> newPages;
        for (; guestsBegin != guestsEnd; ++guestsBegin) {
            for (const int page : (*guestsBegin)->pages) {
                if (!pageFreq.contains(page)) {
                    newPages.insert(page);
                }
            }
        }
        return newPages.size() + pageFreq.size();
    }

    template <typename GuestProfitIt,
              typename K = std::shared_ptr<const Guest>, typename V = int>
        requires PairIterator<GuestProfitIt, K, V>
    [[nodiscard]] size_t countPagesWithGuests(GuestProfitIt guestsBegin,
                                              GuestProfitIt guestsEnd) const
    {
        std::set<int> newPages;
        for (; guestsBegin != guestsEnd; ++guestsBegin) {
            for (const int page : guestsBegin->first->pages) {
                if (!pageFreq.contains(page)) {
                    newPages.insert(page);
                }
            }
        }
        return newPages.size() + pageFreq.size();
    }

    [[nodiscard]] size_t countPagesWithGuest(const Guest &guest) const;

    [[nodiscard]] size_t guestCount() const;
    [[nodiscard]] bool isOverfull() const;
    [[nodiscard]] bool
    hasGuest(const std::shared_ptr<const Guest> &guest) const;

    bool addGuest(const std::shared_ptr<const Guest> &guest);
    bool removeGuest(const std::shared_ptr<const Guest> &guest);
    void clearGuests();

    const size_t capacity;
    std::set<std::shared_ptr<const Guest>> guests;

  private:
    std::map<int, int> pageFreq;
};

}  // namespace vmp

#endif  // SOLVERS_HOST_H

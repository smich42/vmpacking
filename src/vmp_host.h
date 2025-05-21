#ifndef SOLVERS_HOST_H
#define SOLVERS_HOST_H

#include <vmp_commontypes.h>

#include <unordered_map>
#include <unordered_set>
#include <vmp_guest.h>

namespace vmp
{

class Host
{
  public:
    explicit Host(size_t capacity);

    /**
     * Ask if the pages of a guest can be added to the host without exceeding
     * `capacity`
     *
     * @param guest the guest
     * @return true if the host is not overfull after adding the guest
     */
    [[nodiscard]] bool accommodatesGuest(const Guest &guest) const;

    /**
     * Ask if the pages of a set of guests can be added to the host without
     * exceeding `capacity`
     *
     * @param guestsBegin the start of the guest range, inclusive
     * @param guestsEnd the end of the guest range, exclusive
     * @return true if the host is not overfull after adding the guests
     */
    template <SharedPtrIterator<const Guest> GuestIt>
    bool accommodatesGuests(GuestIt guestsBegin, GuestIt guestsEnd) const
    {
        return countPagesWithGuests(guestsBegin, guestsEnd) <= capacity;
    }

    /**
     * Count the number of *unique* pages on this host but not the guest
     *
     * @param guest the guest
     * @return the number of pages
     */
    [[nodiscard]] size_t countPagesNotOn(const Guest &guest) const;

    /**
     * Count the number of *unique* pages this host shares with the guest
     *
     * @param guest the guest
     * @return the number of pages
     */
    [[nodiscard]] size_t countPagesWithGuest(const Guest &guest) const;

    /**
     * Count the number of *unique* pages this host shares with the range of
     * guests
     *
     * @tparam GuestIt any iterator type over `std::shared_ptr<const Guest>`
     * @param guestsBegin the start of the guest range
     * @param guestsEnd the end of the guest range
     * @return
     */
    template <SharedPtrIterator<const Guest> GuestIt>
    [[nodiscard]] size_t countPagesWithGuests(GuestIt guestsBegin, GuestIt guestsEnd) const
    {
        std::unordered_set<int> newPages;
        for (; guestsBegin != guestsEnd; ++guestsBegin) {
            for (const int page : (*guestsBegin)->pages) {
                if (!pageFrequencies.contains(page)) {
                    newPages.insert(page);
                }
            }
        }
        return newPages.size() + pageFrequencies.size();
    }

    /**
     * Get the number of guests on this host that share a page
     *
     * @param page the page
     * @return the number of guests
     */
    [[nodiscard]] size_t getPageFrequency(int page) const;

    /**
     * Get a mapping of each page to the number of guests on this host that share it
     *
     * @return the number of guests
     */
    [[nodiscard]] const std::unordered_map<int, int> &getPageFrequencies() const;

    /**
     * The number of *unique* pages on this host
     *
     * @return the number of *unique* pages on this host
     */
    [[nodiscard]] size_t getUniquePageCount() const;
    [[nodiscard]] size_t getGuestCount() const;
    [[nodiscard]] bool isOverfull() const;
    [[nodiscard]] bool hasGuest(const std::shared_ptr<const Guest> &guest) const;

    bool addGuest(const std::shared_ptr<const Guest> &guest);
    bool removeGuest(const std::shared_ptr<const Guest> &guest);
    void clearGuests();

    [[nodiscard]] size_t getCapacity() const;
    [[nodiscard]] const std::unordered_set<std::shared_ptr<const Guest>> &getGuests() const;

    friend std::ostream &operator<<(std::ostream &os, const Host &host);

    template <SharedPtrIterator<const Guest> GuestIt>
    void addGuests(GuestIt guestsBegin, const GuestIt guestsEnd)
    {
        for (; guestsBegin != guestsEnd; ++guestsBegin) {
            addGuest(*guestsBegin);
        }
    }

  private:
    // Store page frequencies as the number of guests that have a page is
    // useful some Grange heuristics
    std::unordered_map<int, int> pageFrequencies;

    const size_t capacity;
    std::unordered_set<std::shared_ptr<const Guest>> guests;
};

}  // namespace vmp

#endif  // SOLVERS_HOST_H

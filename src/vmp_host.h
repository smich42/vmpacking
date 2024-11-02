#ifndef SOLVERS_HOST_H
#define SOLVERS_HOST_H

#include <map>
#include <set>
#include <vmp_guest.h>

namespace vmp
{

class Host
{
  public:
    explicit Host(int capacity);

    [[nodiscard]] bool accommodatesGuest(const Guest &guest) const;
    [[nodiscard]] size_t pageFrequency(int page) const;
    [[nodiscard]] size_t pageCount() const;
    [[nodiscard]] size_t pageCountWith(const Guest &guest) const;
    [[nodiscard]] size_t guestCount() const;
    [[nodiscard]] bool isOverfull() const;
    bool addGuest(const std::shared_ptr<Guest> &guest);
    bool removeGuest(const std::shared_ptr<Guest> &guest);
    void clearGuests();

    const int capacity;
    std::set<std::shared_ptr<Guest>> guests;

  private:
    std::map<int, int> pageFreq;
};

}  // namespace vmp

#endif  // SOLVERS_HOST_H

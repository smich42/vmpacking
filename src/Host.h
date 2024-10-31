#ifndef HOST_H
#define HOST_H

#include <Guest.h>
#include <unordered_set>

class Host
{
  public:
    explicit Host(int capacity);

    [[nodiscard]] int
    accommodatesGuest(const std::shared_ptr<Guest> &guest) const;

    int addGuest(const std::shared_ptr<Guest> &guest);

    const int capacity;
    std::vector<std::shared_ptr<Guest>> guests;
    std::unordered_map<int, int> pageFreq;
};

#endif  // HOST_H

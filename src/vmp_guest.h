#ifndef SOLVERS_GUEST_H
#define SOLVERS_GUEST_H

#include <set>

namespace vmp
{

class Host;

class Guest
{
  public:
    explicit Guest(const std::set<int> &pages);
    [[nodiscard]] size_t pageCount() const;
    [[nodiscard]] size_t countPagesOn(const Host &host) const;

    const std::set<int> pages;
};

}  // namespace vmp

#endif  // SOLVERS_GUEST_H
#ifndef GUEST_H
#define GUEST_H

#include <unordered_set>

class Host;

class Guest
{
  public:
    const std::unordered_set<int> pages;

    explicit Guest(const std::unordered_set<int> &pages);

    [[nodiscard]] double relativeSizeOn(const Host &host) const;

    [[nodiscard]] size_t pagesOn(const Host &host) const;
};

#endif  // GUEST_H
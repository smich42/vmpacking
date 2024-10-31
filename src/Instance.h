#ifndef INSTANCE_H
#define INSTANCE_H

#include <Guest.h>
#include <vector>

class Instance
{
  public:
    explicit Instance(size_t capacity,
                      const std::vector<std::shared_ptr<Guest>> &guests);

    const size_t capacity;
    const std::vector<std::shared_ptr<Guest>> guests;
};

#endif  // INSTANCE_H
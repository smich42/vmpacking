#include <Instance.h>

Instance::Instance(const size_t capacity,
                   const std::vector<std::shared_ptr<Guest>> &guests)
    : capacity(capacity), guests(guests)
{
}

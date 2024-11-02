#ifndef VMP_ITERATORS_H
#define VMP_ITERATORS_H

#include <vmp_guest.h>

namespace vmp
{
template <typename It, typename T>
concept SharedPtrIterator =
    std::input_iterator<It> &&
    std::same_as<std::iter_value_t<It>, std::shared_ptr<T>>;
}

#endif  // VMP_ITERATORS_H

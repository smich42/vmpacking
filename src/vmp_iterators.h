#ifndef VMP_ITERATORS_H
#define VMP_ITERATORS_H

#include <vmp_guest.h>

#include <memory>

namespace vmp
{

template <typename It, typename T>
concept SharedPtrIterator =
    std::input_iterator<It> && std::same_as<std::iter_value_t<It>, std::shared_ptr<T>>;

template <typename It, typename K, typename V>
concept PairIterator =
    std::input_iterator<It> && std::same_as<std::iter_value_t<It>, std::pair<K, V>>;

using GuestProfitVecIt = std::vector<std::pair<std::shared_ptr<const Guest>, int>>::iterator;

}  // namespace vmp

#endif  // VMP_ITERATORS_H

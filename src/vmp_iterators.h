#ifndef VMP_ITERATORS_H
#define VMP_ITERATORS_H

#include <memory>

namespace vmp
{
template <typename It, typename T>
concept SharedPtrIterator =
    std::input_iterator<It> &&
    std::same_as<std::iter_value_t<It>, std::shared_ptr<T>>;

template <typename It, typename K, typename V>
concept PairIterator = std::input_iterator<It> &&
                       std::same_as<std::iter_value_t<It>, std::pair<K, V>>;

}  // namespace vmp

#endif  // VMP_ITERATORS_H

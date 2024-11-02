#include <vmp_instance.h>

#include <sstream>

namespace vmp
{

Instance::Instance(const size_t capacity,
                   const std::vector<std::shared_ptr<Guest>> &guests)
    : capacity(capacity), guests(guests)
{
}

std::ostream &operator<<(std::ostream &os, const Instance &instance)
{
    os << "Instance{capacity=" << instance.capacity << ", guests=[";
    for (size_t i = 0; i < instance.guests.size(); ++i) {
        if (i > 0) {
            os << ", ";
        }
        os << "{";
        const auto &pages = instance.guests[i]->pages;
        for (auto it = pages.begin(); it != pages.end(); ++it) {
            if (it != pages.begin()) {
                os << ",";
            }
            os << *it;
        }
        os << "}";
    }
    os << "]}";
    return os;
}

}  // namespace vmp
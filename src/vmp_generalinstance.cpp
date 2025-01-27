#include <vmp_generalinstance.h>

#include <sstream>

namespace vmp
{

GeneralInstance::GeneralInstance(const size_t capacity,
                                 const std::vector<std::shared_ptr<const Guest>> &guests)
    : capacity(capacity), guests(guests)
{
}

size_t GeneralInstance::getGuestCount() const
{
    return guests.size();
}

const std::vector<std::shared_ptr<const Guest>> &GeneralInstance::getGuests() const
{
    return guests;
}

size_t GeneralInstance::getCapacity() const
{
    return capacity;
}

std::ostream &operator<<(std::ostream &os, const GeneralInstance &instance)
{
    os << "Instance{ capacity=" << instance.getCapacity() << ", guests=[";
    for (size_t i = 0; i < instance.getGuests().size(); ++i) {
        if (i > 0) {
            os << ", ";
        }
        os << "{";
        const auto &pages = instance.getGuests()[i]->pages;
        for (auto it = pages.begin(); it != pages.end(); ++it) {
            if (it != pages.begin()) {
                os << ",";
            }
            os << *it;
        }
        os << "}";
    }
    os << "] }";
    return os;
}

}  // namespace vmp
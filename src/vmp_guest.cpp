#include <vmp_guest.h>

#include <numeric>
#include <ostream>
#include <vmp_host.h>

namespace vmp
{

Guest::Guest(const std::set<int> &pages) : pages(pages) {}

size_t Guest::getPageCount() const
{
    return pages.size();
}

size_t Guest::countPagesOn(const Host &host) const
{
    return std::ranges::count_if(pages, [&](const int page) { return host.getPageFrequency(page); });
}

std::ostream &operator<<(std::ostream &os, const Guest &instance)
{
    os << "Guest{ pages=[";
    if (!instance.pages.empty()) {
        auto it = instance.pages.begin();
        os << *it;
        while (++it != instance.pages.end()) {
            os << ", " << *it;
        }
    }
    os << "] (len: " << instance.getPageCount() << ") }";
    return os;
}

}  // namespace vmp
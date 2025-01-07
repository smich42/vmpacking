#include <vmp_guest.h>

#include <numeric>
#include <vmp_host.h>

namespace vmp
{

Guest::Guest(const std::set<int> &pages) : pages(pages) {}

size_t Guest::pageCount() const
{
    return pages.size();
}

size_t Guest::countPagesOn(const Host &host) const
{
    return std::ranges::count_if(
        pages,
        [&](const int page) { return host.pageFrequency(page); });
}

}  // namespace vmp
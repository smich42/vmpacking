#include <Guest.h>

#include <Host.h>
#include <numeric>

Guest::Guest(const std::unordered_set<int> &pages) : pages(pages) {}

double Guest::relativeSizeOn(const Host &host) const
{
    return 1.0 / std::accumulate(pages.begin(), pages.end(), 0.0,
                                 [&](const double sum, const int page) {
                                     return sum + (host.pageFreq.contains(page)
                                                       ? host.pageFreq.at(page)
                                                       : 0);
                                 });
}

size_t Guest::pagesOn(const Host &host) const
{
    return std::ranges::count_if(
        pages.begin(), pages.end(), [&](const int page) {
            return host.pageFreq.contains(page) && host.pageFreq.at(page) > 0;
        });
}

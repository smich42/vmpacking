#include <vmp_solverutils.h>

#include <numeric>
#include <unordered_map>
#include <vmp_guest.h>
#include <vmp_host.h>

namespace vmp
{

double calculateRelSize(const Guest &guest,
                        const std::unordered_map<int, int> &pageFreq)
{
    return std::accumulate(guest.pages.begin(), guest.pages.end(), 0.0,
                           [&](const double sum, const int page) {
                               return sum + 1.0 / static_cast<double>(
                                                      pageFreq.at(page));
                           });
}

double calculateSizeRelRatio(const Guest &guest,
                             const std::unordered_map<int, int> &pageFreq)
{
    return static_cast<double>(guest.pageCount()) /
           calculateRelSize(guest, pageFreq);
}

double
calculateLocalityScore(const Guest &guest, const std::shared_ptr<Host> &host,
                       const std::vector<std::shared_ptr<Host>> &allHosts)
{
    const size_t pagesOnHost = guest.countPagesOn(*host);

    size_t minDifferenceWithOtherHost = std::numeric_limits<size_t>::max();
    for (const auto &otherHost : allHosts) {
        if (host != otherHost) {
            minDifferenceWithOtherHost = std::min(
                minDifferenceWithOtherHost, otherHost->countPagesNotOn(guest));
        }
    }

    return static_cast<double>(pagesOnHost + minDifferenceWithOtherHost) /
           std::sqrt(guest.pageCount());
}

double
countGuestPagesPlaced(const Guest &guest,
                      const std::vector<std::shared_ptr<const Host>> &hosts)
{
    const auto frequencies =
        calculatePageFrequencies(hosts.begin(), hosts.end());

    int count = 0;
    for (int page : guest.pages) {
        if (frequencies.contains(page)) {
            ++count;
        }
    }

    return count;
}

}  // namespace vmp
#include <vmp_solverutils.h>

#include <numeric>
#include <unordered_map>
#include <vmp_guest.h>
#include <vmp_host.h>

namespace vmp
{

double relativeSize(const std::shared_ptr<Guest> &guest,
                    const std::unordered_map<int, int> &pageFreq)
{
    return std::accumulate(guest->pages.begin(), guest->pages.end(), 0.0,
                           [&](const double sum, const int page) {
                               return sum + 1.0 / static_cast<double>(
                                                      pageFreq.at(page));
                           });
}

double sizeOverRelativeSize(const std::shared_ptr<Guest> &guest,
                            const std::unordered_map<int, int> &pageFreq)
{
    return static_cast<double>(guest->pageCount()) /
           relativeSize(guest, pageFreq);
}

}  // namespace vmp
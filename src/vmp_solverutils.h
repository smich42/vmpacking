#ifndef VMP_SOLVERUTILS_H
#define VMP_SOLVERUTILS_H

#include <vmp_guest.h>
#include <vmp_iterators.h>

namespace vmp
{

template <SharedPtrIterator<Guest> GuestIt>
std::unordered_map<int, int> calculatePageFrequencies(GuestIt guestsBegin,
                                                      GuestIt guestsEnd)
{
    std::unordered_map<int, int> pageFreq;
    for (; guestsBegin != guestsEnd; ++guestsBegin) {
        for (const auto &page : (*guestsBegin)->pages) {
            ++pageFreq[page];
        }
    }
    return pageFreq;
}

double relativeSize(const std::shared_ptr<Guest> &guest,
                    const std::unordered_map<int, int> &pageFreq);

double sizeOverRelativeSize(const std::shared_ptr<Guest> &guest,
                            const std::unordered_map<int, int> &pageFreq);
}  // namespace vmp

#endif  // VMP_SOLVERUTILS_H
